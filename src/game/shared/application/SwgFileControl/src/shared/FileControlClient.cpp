// ======================================================================
//
// FileControlClient.cpp
// copyright 2024 Sony Online Entertainment
//
// ======================================================================

#include "FirstSwgFileControl.h"
#include "FileControlClient.h"

#include "ConfigFileControl.h"
#include "FileControlConnection.h"

#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Os.h"
#include "sharedLog/Log.h"
#include "sharedNetwork/NetworkSetupData.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <sys/stat.h>

#if defined(PLATFORM_WIN32)
#include <io.h>
#include <windows.h>
#define STAT_FUNC _stat
#define STAT_STRUCT struct _stat
#else
#include <unistd.h>
#define STAT_FUNC stat
#define STAT_STRUCT struct stat
#endif

// ======================================================================

FileControlConnection * FileControlClient::ms_connection    = 0;
bool                    FileControlClient::ms_connected     = false;
bool                    FileControlClient::ms_authenticated = false;
bool                    FileControlClient::ms_done          = false;
FileControlClient::ClientCommand FileControlClient::ms_pendingCommand = CC_NONE;
std::string             FileControlClient::ms_pendingPath;

// ======================================================================

void FileControlClient::install()
{
	ExitChain::add(remove, "FileControlClient::remove");
}

// ----------------------------------------------------------------------

void FileControlClient::remove()
{
	if (ms_connection)
	{
		ms_connection->disconnect();
		ms_connection = 0;
	}
	ms_connected = false;
	ms_authenticated = false;
}

// ----------------------------------------------------------------------

void FileControlClient::update()
{
	if (ms_done)
		return;

	if (!ms_connected)
	{
		if (!connect())
		{
			ms_done = true;
			return;
		}
	}
}

// ======================================================================
// Connection
// ======================================================================

bool FileControlClient::connect()
{
	const char * host = ConfigFileControl::getClientHost();
	int port = ConfigFileControl::getClientPort();

	printf("Connecting to %s:%d...\n", host, port);
	LOG("FileControl", ("Connecting to %s:%d", host, port));

	NetworkSetupData setupData;
	setupData.useTcp = true;

	ms_connection = new FileControlConnection(host, static_cast<unsigned short>(port), setupData);

	if (!ms_connection)
	{
		printf("ERROR: Failed to create connection\n");
		return false;
	}

	ms_connected = true;

	if (!authenticate())
	{
		printf("ERROR: Authentication failed\n");
		ms_done = true;
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------

bool FileControlClient::authenticate()
{
	if (!ms_connection)
		return false;

	const char * key = ConfigFileControl::getClientFileServerKey();
	if (!key || key[0] == '\0')
	{
		printf("ERROR: No fileServerKey configured in [ClientGame]\n");
		return false;
	}

	std::string machineId = getMachineId();
	ms_connection->sendAuthRequest(key, machineId);

	LOG("FileControl", ("Authentication request sent (machineId=%s)", machineId.c_str()));
	return true;
}

// ======================================================================
// Command execution
// ======================================================================

bool FileControlClient::executeCommand(ClientCommand command, const std::string & filePath)
{
	switch (command)
	{
	case CC_UPDATE_FILE:
		return doUpdateFile(filePath);
	case CC_DOWNLOAD_FILE:
		return doDownloadFile(filePath);
	case CC_COMPARE_FILE:
		return doCompareFile(filePath);
	case CC_TEST_RUN:
		return doTestRun(filePath);
	default:
		printf("ERROR: Unknown command\n");
		return false;
	}
}

// ----------------------------------------------------------------------

bool FileControlClient::doUpdateFile(const std::string & filePath)
{
	printf("Uploading: %s\n", filePath.c_str());

	std::vector<unsigned char> data;
	if (!readLocalFile(filePath, data))
	{
		printf("ERROR: Cannot read local file: %s\n", filePath.c_str());
		return false;
	}

	printf("File size: %d bytes\n", static_cast<int>(data.size()));

	if (!ms_connection || !ms_connected)
	{
		printf("ERROR: Not connected to server\n");
		return false;
	}

	bool compress = ConfigFileControl::getClientEnableCompression();
	ms_connection->sendFileData(filePath, data, compress);

	printf("File sent to server. Waiting for confirmation...\n");
	LOG("FileControl", ("Uploaded %s (%d bytes, compressed=%s)", filePath.c_str(), static_cast<int>(data.size()), compress ? "yes" : "no"));

	return true;
}

// ----------------------------------------------------------------------

bool FileControlClient::doDownloadFile(const std::string & filePath)
{
	printf("Requesting download: %s\n", filePath.c_str());

	if (!ms_connection || !ms_connected)
	{
		printf("ERROR: Not connected to server\n");
		return false;
	}

	ms_connection->sendFileRequest(filePath);
	LOG("FileControl", ("Download requested: %s", filePath.c_str()));

	return true;
}

// ----------------------------------------------------------------------

bool FileControlClient::doCompareFile(const std::string & filePath)
{
	printf("Comparing: %s\n", filePath.c_str());

	if (!ms_connection || !ms_connected)
	{
		printf("ERROR: Not connected to server\n");
		return false;
	}

	unsigned long localSize = getLocalFileSize(filePath);
	unsigned long localCrc = getLocalFileCrc(filePath);

	printf("Local:  size=%lu  crc=0x%08lx\n", localSize, localCrc);

	ms_connection->sendCompareRequest(filePath);
	LOG("FileControl", ("Compare requested: %s (local size=%lu, crc=0x%08lx)", filePath.c_str(), localSize, localCrc));

	return true;
}

// ----------------------------------------------------------------------

bool FileControlClient::doTestRun(const std::string & filePath)
{
	printf("Test run: %s\n", filePath.c_str());
	printf("Testing network connectivity (no transfer)...\n");

	if (!ms_connection || !ms_connected)
	{
		printf("ERROR: Not connected to server\n");
		return false;
	}

	ms_connection->sendTestRequest(filePath);

	printf("Connection: OK\n");
	printf("Server:     %s:%d\n", ConfigFileControl::getClientHost(), ConfigFileControl::getClientPort());
	printf("Auth:       %s\n", ms_authenticated ? "authenticated" : "pending");

	LOG("FileControl", ("Test run for %s - connectivity OK", filePath.c_str()));

	return true;
}

// ======================================================================
// File I/O
// ======================================================================

bool FileControlClient::readLocalFile(const std::string & filePath, std::vector<unsigned char> & data)
{
	FILE * fp = fopen(filePath.c_str(), "rb");
	if (!fp)
		return false;

	fseek(fp, 0, SEEK_END);
	long fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if (fileSize <= 0)
	{
		fclose(fp);
		data.clear();
		return true;
	}

	data.resize(static_cast<size_t>(fileSize));
	size_t bytesRead = fread(&data[0], 1, static_cast<size_t>(fileSize), fp);
	fclose(fp);

	return bytesRead == static_cast<size_t>(fileSize);
}

// ----------------------------------------------------------------------

bool FileControlClient::writeLocalFile(const std::string & filePath, const std::vector<unsigned char> & data)
{
	FILE * fp = fopen(filePath.c_str(), "wb");
	if (!fp)
		return false;

	if (!data.empty())
	{
		size_t written = fwrite(&data[0], 1, data.size(), fp);
		fclose(fp);
		return written == data.size();
	}

	fclose(fp);
	return true;
}

// ----------------------------------------------------------------------

unsigned long FileControlClient::getLocalFileCrc(const std::string & filePath)
{
	std::vector<unsigned char> data;
	if (!readLocalFile(filePath, data) || data.empty())
		return 0;

	return Crc::calculate(&data[0], static_cast<int>(data.size()));
}

// ----------------------------------------------------------------------

unsigned long FileControlClient::getLocalFileSize(const std::string & filePath)
{
	STAT_STRUCT st;
	if (STAT_FUNC(filePath.c_str(), &st) != 0)
		return 0;
	return static_cast<unsigned long>(st.st_size);
}

// ----------------------------------------------------------------------

std::string FileControlClient::getMachineId()
{
#if defined(PLATFORM_WIN32)
	char computerName[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD size = sizeof(computerName);
	if (GetComputerNameA(computerName, &size))
		return std::string(computerName);
	return "unknown-win32";
#else
	char hostname[256];
	if (gethostname(hostname, sizeof(hostname)) == 0)
		return std::string(hostname);
	return "unknown-linux";
#endif
}

// ======================================================================
