// ======================================================================
//
// CityTerrainLayerManager.h
// copyright 2026 Titan
//
// Client-side manager for city terrain modifications (shader overlays, flattening)
// ======================================================================

#ifndef INCLUDED_CityTerrainLayerManager_H
#define INCLUDED_CityTerrainLayerManager_H

#include "sharedFoundation/NetworkId.h"
#include "sharedMath/Vector.h"
#include "sharedMath/Vector2d.h"
#include <map>
#include <vector>
#include <string>

class Shader;
class TerrainGenerator;

// ======================================================================

class CityTerrainLayerManager
{
public:
	// Terrain region types
	enum RegionType
	{
		RT_SHADER_CIRCLE = 0,
		RT_SHADER_LINE = 1,
		RT_FLATTEN = 2
	};

	// Structure representing a terrain modification region
	struct TerrainRegion
	{
		std::string regionId;
		int32 cityId;
		RegionType type;
		std::string shaderTemplate;
		float centerX;
		float centerZ;
		float radius;
		float endX;
		float endZ;
		float width;
		float height;
		float blendDistance;
		Shader const * cachedShader;
		bool active;
		int64 timestamp;  // Creation timestamp for priority ordering (newer = higher priority)
		int32 priority;   // Explicit priority (higher = takes precedence)
	};

public:
	static void install();
	static void remove();
	static CityTerrainLayerManager & getInstance();
	static bool isInstalled();

	// Pending city ID for UI activation (set by GameNetwork, read by SwgCuiCityTerrainPainter)
	static void setPendingCityId(int32 cityId);
	static int32 getPendingCityId();

	// City radius tracking (for UI use)
	static void setCityRadius(int32 cityId, int32 radius);
	static int32 getCityRadius(int32 cityId);

	// Region management
	void addRegion(const TerrainRegion & region);
	void removeRegion(const std::string & regionId);
	void removeAllRegionsForCity(int32 cityId);
	void clearAllRegions();

	// Query regions
	bool hasRegion(const std::string & regionId) const;
	const TerrainRegion * getRegion(const std::string & regionId) const;
	void getRegionsAtLocation(float x, float z, std::vector<const TerrainRegion *> & outRegions) const;
	void getRegionsForCity(int32 cityId, std::vector<const TerrainRegion *> & outRegions) const;

	// Shader enumeration - dynamically list all available terrain shaders
	void enumerateAvailableShaders(std::vector<std::string> & outTemplates, std::vector<std::string> & outNames) const;

	// Height modification for flatten regions (instance method)
	bool getModifiedHeightInternal(float x, float z, float originalHeight, float & outHeight) const;

	// Shader blending query - returns shader override if in a modified region (instance method)
	bool getShaderOverrideInternal(float x, float z, Shader const *& outShader, float & outBlendWeight) const;

	// Static wrappers that check if instance exists
	static bool getModifiedHeight(float x, float z, float originalHeight, float & outHeight);
	static bool getShaderOverride(float x, float z, Shader const *& outShader, float & outBlendWeight);

	// Update/render callback
	void update(float elapsedTime);

	// Sync from server
	void handleTerrainModifyMessage(int32 cityId, int32 modificationType, const std::string & regionId,
		const std::string & shaderTemplate, float centerX, float centerZ, float radius,
		float endX, float endZ, float width, float height, float blendDistance);
	void handleTerrainSyncMessage(int32 cityId, const std::vector<TerrainRegion> & regions);

	// Force terrain regeneration for all modified areas to prevent edge gaps
	void flushTerrainUpdates();

	// Callback for UI to be notified when regions change
	typedef void (*RegionChangeCallback)(int32 cityId);
	static void setRegionChangeCallback(RegionChangeCallback callback);
	static void clearRegionChangeCallback();

private:
	CityTerrainLayerManager();
	~CityTerrainLayerManager();
	CityTerrainLayerManager(const CityTerrainLayerManager &);
	CityTerrainLayerManager & operator=(const CityTerrainLayerManager &);

	void loadShaderForRegion(TerrainRegion & region);
	bool isPointInCircle(float px, float pz, float cx, float cz, float radius) const;
	bool isPointInLine(float px, float pz, float x1, float z1, float x2, float z2, float width) const;
	float calculateBlendWeight(float distance, float blendDistance) const;

private:
	static CityTerrainLayerManager * ms_instance;
	static int32 ms_pendingCityId;
	static std::map<int32, int32> ms_cityRadii;
	static RegionChangeCallback ms_regionChangeCallback;

	typedef std::map<std::string, TerrainRegion> RegionMap;
	RegionMap m_regions;

	typedef std::multimap<int32, std::string> CityRegionMap;
	CityRegionMap m_cityRegions;

	// Cached shader list
	mutable std::vector<std::string> m_cachedShaderTemplates;
	mutable std::vector<std::string> m_cachedShaderNames;
	mutable bool m_shaderListDirty;
};

// ======================================================================

#endif // INCLUDED_CityTerrainLayerManager_H

