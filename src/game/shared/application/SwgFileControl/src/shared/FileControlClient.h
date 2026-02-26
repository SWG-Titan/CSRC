// ======================================================================
//
// FileControlClient.h
// copyright 2024 Sony Online Entertainment
//
// Client mode for SwgFileControl - connects to server, executes
// commands (--update-file, --download-file, --compare-file, --test-run).
//
// ======================================================================

#ifndef INCLUDED_FileControlClient_H
#define INCLUDED_FileControlClient_H

// ======================================================================

#include <string>
#include <vector>

class FileControlConnection;

// ======================================================================

class FileControlClient
{
public:

	enum ClientCommand
	{
		CC_NONE,
		CC_UPDATE_FILE,
		CC_DOWNLOAD_FILE,
		CC_COMPARE_FILE,
		CC_TEST_RUN
	};

	static void install();
	static void remove();
	static void update();

	static bool executeCommand(ClientCommand command, const std::string & filePath);
	static bool isConnected();
	static bool isDone();

private:

	FileControlClient();
	FileControlClient(const FileControlClient &);
	FileControlClient & operator=(const FileControlClient &);

	static bool connect();
	static bool authenticate();
	static bool readLocalFile(const std::string & filePath, std::vector<unsigned char> & data);
	static bool writeLocalFile(const std::string & filePath, const std::vector<unsigned char> & data);
	static unsigned long getLocalFileCrc(const std::string & filePath);
	static unsigned long getLocalFileSize(const std::string & filePath);
	static std::string getMachineId();

	static bool doUpdateFile(const std::string & filePath);
	static bool doDownloadFile(const std::string & filePath);
	static bool doCompareFile(const std::string & filePath);
	static bool doTestRun(const std::string & filePath);

	static FileControlConnection * ms_connection;
	static bool                    ms_connected;
	static bool                    ms_authenticated;
	static bool                    ms_done;
	static ClientCommand           ms_pendingCommand;
	static std::string             ms_pendingPath;
};

// ======================================================================

inline bool FileControlClient::isConnected()
{
	return ms_connected;
}

inline bool FileControlClient::isDone()
{
	return ms_done;
}

// ======================================================================

#endif
