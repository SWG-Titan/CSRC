// ======================================================================
//
// FileControlReloadHandler.h
// copyright 2024 Sony Online Entertainment
//
// Handles RELOAD_NOTIFY messages from SwgFileControl by invoking
// the actual game engine reload mechanisms. Client-side version
// uses client reload APIs (DataTableManager, ObjectTemplateList, etc.)
//
// ======================================================================

#ifndef INCLUDED_FileControlReloadHandler_H
#define INCLUDED_FileControlReloadHandler_H

// ======================================================================

#include "HotReloadManager.h"

#include <string>
#include <vector>

// ======================================================================

class FileControlReloadHandler
{
public:

	static void install();
	static void remove();

	static bool handleReloadNotify(const std::string & relativePath, const std::string & reloadCommand);

	static bool reloadScript(const std::string & scriptName);
	static bool reloadDatatable(const std::string & datatableName);
	static bool reloadTemplate(const std::string & templatePath);
	static bool reloadBuildout(const std::string & buildoutPath);
	static bool reloadGenericAsset(const std::string & assetPath);

	static bool handleReceivedFile(const std::string & relativePath, const std::vector<unsigned char> & data);

private:

	FileControlReloadHandler();
	FileControlReloadHandler(const FileControlReloadHandler &);
	FileControlReloadHandler & operator=(const FileControlReloadHandler &);

	static bool ms_installed;
};

// ======================================================================

#endif
