// ======================================================================
//
// FileControlServer.h
// copyright 2024 Sony Online Entertainment
//
// Server mode for SwgFileControl - listens for connections, manages
// file uploads/downloads, triggers hot reloads, and distributes
// updates to connected clients.
//
// ======================================================================

#ifndef INCLUDED_FileControlServer_H
#define INCLUDED_FileControlServer_H

// ======================================================================

#include "FileControlQueue.h"
#include "HotReloadManager.h"

#include <string>
#include <vector>
#include <set>
#include <map>

class FileControlConnection;
class Service;
class NetworkSetupData;

// ======================================================================

class FileControlServer
{
public:

	static void install();
	static void remove();
	static void update();

	static bool isRunning();
	static const std::string & getChannel();
	static int  getConnectedClientCount();

	// Public API for GodClient integration
	static bool requestSendAsset(const std::string & relativePath);
	static bool requestRetrieveAsset(const std::string & relativePath, std::vector<unsigned char> & outData);
	static bool requestReloadAsset(const std::string & relativePath);
	static bool requestBroadcastUpdate();
	static bool requestUpdateDbTemplates();
	static bool requestVerifyAsset(const std::string & relativePath, unsigned long & outSize, unsigned long & outCrc);
	static bool requestFlush();
	static bool requestDirectoryListing(const std::string & rootPath, std::vector<std::string> & outFiles, std::vector<unsigned long> & outSizes, std::vector<unsigned long> & outCrcs);

	// Connection callbacks (called from FileControlConnection)
	static void onConnectionOpened(FileControlConnection * connection);
	static void onConnectionClosed(FileControlConnection * connection);
	static void onFileUploadReceived(FileControlConnection * connection, const std::string & relativePath, const std::vector<unsigned char> & data, bool compressed);
	static void onFileDownloadRequested(FileControlConnection * connection, const std::string & relativePath);
	static void onFileCompareRequested(FileControlConnection * connection, const std::string & relativePath);
	static void onFileTestRequested(FileControlConnection * connection, const std::string & relativePath);
	static void onFileListRequested(FileControlConnection * connection, const std::string & rootPath);

private:

	FileControlServer();
	FileControlServer(const FileControlServer &);
	FileControlServer & operator=(const FileControlServer &);

	static void processQueue();
	static bool handleFileUpload(const FileControlQueue::QueueEntry & entry);
	static bool handleFileDownload(const FileControlQueue::QueueEntry & entry);
	static bool handleFileCompare(const FileControlQueue::QueueEntry & entry);

	static bool storeFile(const std::string & relativePath, const std::vector<unsigned char> & data);
	static bool loadFile(const std::string & relativePath, std::vector<unsigned char> & data);
	static bool backupFile(const std::string & relativePath);
	static bool rollbackFile(const std::string & relativePath);
	static bool fileExists(const std::string & relativePath);
	static unsigned long getFileSize(const std::string & relativePath);
	static unsigned long getFileCrc(const std::string & relativePath);
	static unsigned long getFileVersion(const std::string & relativePath);

	static std::string getAbsolutePath(const std::string & relativePath);
	static bool        createDirectoryForFile(const std::string & absolutePath);
	static void        collectDirectoryFiles(const std::string & dirPath, const std::string & prefix, std::vector<std::string> & outFiles, std::vector<unsigned long> & outSizes, std::vector<unsigned long> & outCrcs);

	static bool isPathBlacklisted(const std::string & relativePath, bool isUpload);
	static bool isExtensionWhitelisted(const std::string & relativePath);
	static bool validatePath(const std::string & relativePath);
	static bool isDsrcPath(const std::string & relativePath);

	static bool executeHotReload(const std::string & relativePath, const HotReloadManager::ReloadAction & action);
	static bool runAntBuildCrcTable();
	static bool runAntProcessTemplates();
	static bool sendReloadCommandToGameServers(const std::string & command, const std::string & target);
	static void distributeToClients(const std::string & relativePath, const std::vector<unsigned char> & data);
	static void distributeCrcTableToClients();

	static void logAuthFailureToCentral(const std::string & remoteAddress, int remotePort, const std::string & machineId);

	static bool compressData(const std::vector<unsigned char> & input, std::vector<unsigned char> & output);
	static bool decompressData(const std::vector<unsigned char> & input, std::vector<unsigned char> & output, unsigned long originalSize);

	static Service *                          ms_service;
	static FileControlQueue                   ms_queue;
	static std::set<FileControlConnection *>  ms_connections;
	static bool                               ms_running;
	static std::string                        ms_dataRoot;
	static std::string                        ms_channel;
	static std::map<std::string, unsigned long> ms_versionMap;
	static unsigned long                      ms_bytesSentThisSecond;
	static unsigned long                      ms_lastRateLimitTime;
};

// ======================================================================

inline bool FileControlServer::isRunning()
{
	return ms_running;
}

// ======================================================================

#endif
