// ======================================================================
//
// HotReloadManager.cpp
// copyright 2024 Sony Online Entertainment
//
// ======================================================================

#include "FirstSwgFileControl.h"
#include "HotReloadManager.h"

#include "sharedLog/Log.h"

#include <algorithm>
#include <cstring>

// ======================================================================

namespace HotReloadManagerNamespace
{
	bool endsWith(const std::string & str, const std::string & suffix)
	{
		if (suffix.size() > str.size())
			return false;
		return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
	}

	bool containsPath(const std::string & path, const std::string & segment)
	{
		return path.find(segment) != std::string::npos;
	}

	std::string toLower(const std::string & s)
	{
		std::string result = s;
		std::transform(result.begin(), result.end(), result.begin(), ::tolower);
		return result;
	}
}

using namespace HotReloadManagerNamespace;

// ======================================================================

bool HotReloadManager::isSharedPath(const std::string & relativePath)
{
	return containsPath(relativePath, "sys.shared");
}

// ----------------------------------------------------------------------

bool HotReloadManager::isServerPath(const std::string & relativePath)
{
	return containsPath(relativePath, "sys.server");
}

// ----------------------------------------------------------------------

HotReloadManager::FileType HotReloadManager::detectFileType(const std::string & relativePath)
{
	std::string lowerPath = toLower(relativePath);

	if (endsWith(lowerPath, ".class"))
	{
		return FT_JAVA_CLASS;
	}

	if (endsWith(lowerPath, ".iff"))
	{
		if (containsPath(lowerPath, "datatables"))
		{
			if (isSharedPath(lowerPath))
				return FT_DATATABLE_SHARED;
			else
				return FT_DATATABLE_SERVER;
		}

		if (containsPath(lowerPath, "object/"))
		{
			if (isSharedPath(lowerPath))
				return FT_TEMPLATE_SHARED;
			else
				return FT_TEMPLATE_SERVER;
		}

		if (containsPath(lowerPath, "buildout"))
			return FT_BUILDOUT;

		if (containsPath(lowerPath, "string"))
			return FT_STRING_TABLE;

		if (endsWith(lowerPath, ".trn") || containsPath(lowerPath, "terrain"))
			return FT_TERRAIN;

		return FT_OTHER_IFF;
	}

	return FT_UNKNOWN;
}

// ----------------------------------------------------------------------

std::string HotReloadManager::getScriptReloadName(const std::string & relativePath)
{
	// Convert: data/sku.0/sys.server/compiled/game/script/bubbajoe/cmd.class
	// To:      bubbajoe.cmd

	std::string result = relativePath;

	// Find "script/" and take everything after it
	std::string::size_type scriptPos = result.find("script/");
	if (scriptPos == std::string::npos)
		scriptPos = result.find("script\\");
	if (scriptPos != std::string::npos)
		result = result.substr(scriptPos + 7);

	// Remove .class extension
	std::string::size_type extPos = result.rfind(".class");
	if (extPos != std::string::npos)
		result = result.substr(0, extPos);

	// Replace path separators with dots
	for (std::string::size_type i = 0; i < result.size(); ++i)
	{
		if (result[i] == '/' || result[i] == '\\')
			result[i] = '.';
	}

	return result;
}

// ----------------------------------------------------------------------

std::string HotReloadManager::getDatatableReloadName(const std::string & relativePath)
{
	// Convert: data/sku.0/sys.server/compiled/game/datatables/dungeon/fort_tusken.iff
	// To:      datatables/dungeon/fort_tusken.iff

	std::string result = relativePath;

	std::string::size_type dtPos = result.find("datatables");
	if (dtPos != std::string::npos)
		result = result.substr(dtPos);

	// Normalize separators
	for (std::string::size_type i = 0; i < result.size(); ++i)
	{
		if (result[i] == '\\')
			result[i] = '/';
	}

	return result;
}

// ----------------------------------------------------------------------

bool HotReloadManager::isQuestRelatedDatatable(const std::string & relativePath)
{
	std::string lowerPath = toLower(relativePath);
	return containsPath(lowerPath, "questlist") || containsPath(lowerPath, "questtask");
}

// ----------------------------------------------------------------------

bool HotReloadManager::determineReloadAction(const std::string & relativePath, ReloadAction & action)
{
	FileType ft = detectFileType(relativePath);

	switch (ft)
	{
	case FT_JAVA_CLASS:
		// Example 1: script reload bubbajoe.cmd
		action.reloadCommand = "script reload";
		action.reloadTarget = getScriptReloadName(relativePath);
		action.requiresCrcRebuild = false;
		action.requiresQuestCrcRebuild = false;
		action.sendToClients = false;
		action.reloadOnGameServers = true;
		action.reloadOnPlanetServers = false;
		LOG("FileControl", ("Hot reload: Java class -> script reload %s", action.reloadTarget.c_str()));
		return true;

	case FT_DATATABLE_SERVER:
		// Example 2: server datatable reload
		action.reloadCommand = "datatable reload";
		action.reloadTarget = getDatatableReloadName(relativePath);
		action.requiresCrcRebuild = false;
		action.requiresQuestCrcRebuild = false;
		action.sendToClients = false;
		action.reloadOnGameServers = true;
		action.reloadOnPlanetServers = false;
		LOG("FileControl", ("Hot reload: Server datatable -> reload %s", action.reloadTarget.c_str()));
		return true;

	case FT_DATATABLE_SHARED:
		// Example 3: shared datatable - also send to clients
		action.reloadCommand = "datatable reload";
		action.reloadTarget = getDatatableReloadName(relativePath);
		action.requiresCrcRebuild = false;
		action.requiresQuestCrcRebuild = isQuestRelatedDatatable(relativePath);
		action.sendToClients = true;
		action.reloadOnGameServers = true;
		action.reloadOnPlanetServers = false;
		LOG("FileControl", ("Hot reload: Shared datatable -> reload %s (quest CRC: %s)", action.reloadTarget.c_str(), action.requiresQuestCrcRebuild ? "yes" : "no"));
		return true;

	case FT_TEMPLATE_SERVER:
		// Example 4A: server template - requires CRC table rebuild
		action.reloadCommand = "template reload";
		action.reloadTarget = relativePath;
		action.requiresCrcRebuild = true;
		action.requiresQuestCrcRebuild = false;
		action.sendToClients = true;
		action.reloadOnGameServers = true;
		action.reloadOnPlanetServers = false;
		LOG("FileControl", ("Hot reload: Server template -> CRC rebuild + reload"));
		return true;

	case FT_TEMPLATE_SHARED:
		// Example 4B: shared template - requires CRC table rebuild + client distribution
		action.reloadCommand = "template reload";
		action.reloadTarget = relativePath;
		action.requiresCrcRebuild = true;
		action.requiresQuestCrcRebuild = false;
		action.sendToClients = true;
		action.reloadOnGameServers = true;
		action.reloadOnPlanetServers = true;
		LOG("FileControl", ("Hot reload: Shared template -> CRC rebuild + reload + distribute"));
		return true;

	case FT_BUILDOUT:
		action.reloadCommand = "buildout reload";
		action.reloadTarget = relativePath;
		action.requiresCrcRebuild = false;
		action.requiresQuestCrcRebuild = false;
		action.sendToClients = true;
		action.reloadOnGameServers = true;
		action.reloadOnPlanetServers = true;
		LOG("FileControl", ("Hot reload: Buildout -> reload + redraw world"));
		return true;

	case FT_STRING_TABLE:
	case FT_TERRAIN:
	case FT_OTHER_IFF:
		action.reloadCommand = "asset reload";
		action.reloadTarget = relativePath;
		action.requiresCrcRebuild = false;
		action.requiresQuestCrcRebuild = false;
		action.sendToClients = isSharedPath(relativePath);
		action.reloadOnGameServers = true;
		action.reloadOnPlanetServers = false;
		LOG("FileControl", ("Hot reload: Generic asset -> reload %s", relativePath.c_str()));
		return true;

	case FT_UNKNOWN:
	default:
		LOG("FileControl", ("Hot reload: Unknown file type for %s, no reload action", relativePath.c_str()));
		return false;
	}
}

// ======================================================================
