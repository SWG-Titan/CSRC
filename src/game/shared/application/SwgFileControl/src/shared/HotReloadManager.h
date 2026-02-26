// ======================================================================
//
// HotReloadManager.h
// copyright 2024 Sony Online Entertainment
//
// Detects file types and determines reload strategy.
//
// ======================================================================

#ifndef INCLUDED_HotReloadManager_H
#define INCLUDED_HotReloadManager_H

// ======================================================================

#include <string>
#include <vector>

// ======================================================================

class HotReloadManager
{
public:

	enum FileType
	{
		FT_UNKNOWN,
		FT_JAVA_CLASS,
		FT_DATATABLE_SERVER,
		FT_DATATABLE_SHARED,
		FT_TEMPLATE_SERVER,
		FT_TEMPLATE_SHARED,
		FT_BUILDOUT,
		FT_STRING_TABLE,
		FT_TERRAIN,
		FT_OTHER_IFF
	};

	struct ReloadAction
	{
		std::string reloadCommand;
		std::string reloadTarget;
		bool        requiresCrcRebuild;
		bool        requiresQuestCrcRebuild;
		bool        sendToClients;
		bool        reloadOnGameServers;
		bool        reloadOnPlanetServers;

		ReloadAction() :
			reloadCommand(),
			reloadTarget(),
			requiresCrcRebuild(false),
			requiresQuestCrcRebuild(false),
			sendToClients(false),
			reloadOnGameServers(false),
			reloadOnPlanetServers(false)
		{
		}
	};

	static FileType    detectFileType(const std::string & relativePath);
	static bool        determineReloadAction(const std::string & relativePath, ReloadAction & action);
	static std::string getScriptReloadName(const std::string & relativePath);
	static std::string getDatatableReloadName(const std::string & relativePath);
	static bool        isQuestRelatedDatatable(const std::string & relativePath);
	static bool        isSharedPath(const std::string & relativePath);
	static bool        isServerPath(const std::string & relativePath);

private:

	HotReloadManager();
	HotReloadManager(const HotReloadManager &);
	HotReloadManager & operator=(const HotReloadManager &);
};

// ======================================================================

#endif
