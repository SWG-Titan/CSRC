// ======================================================================
//
// FileControlConnection.h
// copyright 2024 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FileControlConnection_H
#define INCLUDED_FileControlConnection_H

// ======================================================================

#include "sharedNetwork/Connection.h"

#include <string>
#include <vector>

// ======================================================================

class FileControlConnection : public Connection
{
public:

	FileControlConnection(const std::string & remoteAddress, const unsigned short remotePort, const NetworkSetupData & setup);
	FileControlConnection(UdpConnectionMT * u, TcpClient * t);
	virtual ~FileControlConnection();

	virtual void onConnectionClosed();
	virtual void onConnectionOpened();
	virtual void onReceive(const Archive::ByteStream & bs);

	bool         isAuthenticated() const;
	void         setAuthenticated(bool authenticated);

	const std::string & getMachineId() const;
	void                setMachineId(const std::string & machineId);

	enum MessageType
	{
		MT_AUTH_REQUEST      = 0x01,
		MT_AUTH_RESPONSE     = 0x02,
		MT_FILE_UPLOAD       = 0x10,
		MT_FILE_DOWNLOAD_REQ = 0x11,
		MT_FILE_DOWNLOAD_RSP = 0x12,
		MT_FILE_COMPARE_REQ  = 0x13,
		MT_FILE_COMPARE_RSP  = 0x14,
		MT_FILE_TEST_REQ     = 0x15,
		MT_FILE_TEST_RSP     = 0x16,
		MT_FILE_LIST_REQ     = 0x20,
		MT_FILE_LIST_RSP     = 0x21,
		MT_RELOAD_NOTIFY     = 0x30,
		MT_STATUS            = 0x40,
		MT_ERROR             = 0xFF
	};

	void sendAuthRequest(const std::string & key, const std::string & machineId);
	void sendAuthResponse(bool success, const std::string & message);
	void sendFileData(const std::string & relativePath, const std::vector<unsigned char> & data, bool compressed);
	void sendFileRequest(const std::string & relativePath);
	void sendCompareRequest(const std::string & relativePath);
	void sendCompareResponse(const std::string & relativePath, unsigned long localSize, unsigned long localCrc);
	void sendTestRequest(const std::string & relativePath);
	void sendTestResponse(bool reachable, const std::string & info);
	void sendFileListRequest(const std::string & rootPath);
	void sendFileListResponse(const std::vector<std::string> & files, const std::vector<unsigned long> & sizes);
	void sendReloadNotify(const std::string & relativePath, const std::string & reloadType);
	void sendStatus(const std::string & message);
	void sendError(const std::string & message);

private:

	FileControlConnection(const FileControlConnection &);
	FileControlConnection & operator=(const FileControlConnection &);

	bool        m_authenticated;
	std::string m_machineId;
};

// ======================================================================

inline bool FileControlConnection::isAuthenticated() const
{
	return m_authenticated;
}

inline void FileControlConnection::setAuthenticated(bool authenticated)
{
	m_authenticated = authenticated;
}

inline const std::string & FileControlConnection::getMachineId() const
{
	return m_machineId;
}

inline void FileControlConnection::setMachineId(const std::string & machineId)
{
	m_machineId = machineId;
}

// ======================================================================

#endif
