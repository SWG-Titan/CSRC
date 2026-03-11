// ======================================================================
//
// CityTerrainLayerManager.cpp
// copyright 2026 Titan
//
// Client-side manager for city terrain modifications
// ======================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/CityTerrainLayerManager.h"

#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedMath/Rectangle2d.h"
#include "sharedTerrain/TerrainGenerator.h"
#include "sharedTerrain/TerrainObject.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <set>

// ======================================================================

CityTerrainLayerManager * CityTerrainLayerManager::ms_instance = 0;
int32 CityTerrainLayerManager::ms_pendingCityId = 0;
std::map<int32, int32> CityTerrainLayerManager::ms_cityRadii;
CityTerrainLayerManager::RegionChangeCallback CityTerrainLayerManager::ms_regionChangeCallback = 0;

namespace
{
	// Static counter for region priority - newer regions get higher priority
	int32 s_regionPriorityCounter = 0;
}

// ======================================================================

void CityTerrainLayerManager::install()
{
	DEBUG_FATAL(ms_instance != 0, ("CityTerrainLayerManager already installed"));
	ms_instance = new CityTerrainLayerManager();
	ExitChain::add(CityTerrainLayerManager::remove, "CityTerrainLayerManager::remove");
}

// ----------------------------------------------------------------------

void CityTerrainLayerManager::remove()
{
	DEBUG_FATAL(ms_instance == 0, ("CityTerrainLayerManager not installed"));
	delete ms_instance;
	ms_instance = 0;
}

// ----------------------------------------------------------------------

CityTerrainLayerManager & CityTerrainLayerManager::getInstance()
{
	DEBUG_FATAL(ms_instance == 0, ("CityTerrainLayerManager not installed"));
	return *ms_instance;
}

// ----------------------------------------------------------------------

bool CityTerrainLayerManager::isInstalled()
{
	return ms_instance != 0;
}

// ----------------------------------------------------------------------

void CityTerrainLayerManager::setPendingCityId(int32 cityId)
{
	ms_pendingCityId = cityId;
}

// ----------------------------------------------------------------------

int32 CityTerrainLayerManager::getPendingCityId()
{
	int32 result = ms_pendingCityId;
	ms_pendingCityId = 0;
	return result;
}

// ----------------------------------------------------------------------

void CityTerrainLayerManager::setCityRadius(int32 cityId, int32 radius)
{
	ms_cityRadii[cityId] = radius;
}

// ----------------------------------------------------------------------

int32 CityTerrainLayerManager::getCityRadius(int32 cityId)
{
	std::map<int32, int32>::const_iterator it = ms_cityRadii.find(cityId);
	if (it != ms_cityRadii.end())
	{
		return it->second;
	}
	return 150; // Default city radius fallback
}

// ----------------------------------------------------------------------

void CityTerrainLayerManager::setRegionChangeCallback(RegionChangeCallback callback)
{
	ms_regionChangeCallback = callback;
}

// ----------------------------------------------------------------------

void CityTerrainLayerManager::clearRegionChangeCallback()
{
	ms_regionChangeCallback = 0;
}

// ----------------------------------------------------------------------

bool CityTerrainLayerManager::getModifiedHeight(float x, float z, float originalHeight, float & outHeight)
{
	if (!ms_instance)
	{
		outHeight = originalHeight;
		return false;
	}
	return ms_instance->getModifiedHeightInternal(x, z, originalHeight, outHeight);
}

// ----------------------------------------------------------------------

bool CityTerrainLayerManager::getShaderOverride(float x, float z, Shader const *& outShader, float & outBlendWeight)
{
	if (!ms_instance)
	{
		outShader = 0;
		outBlendWeight = 0.0f;
		return false;
	}
	return ms_instance->getShaderOverrideInternal(x, z, outShader, outBlendWeight);
}

// ----------------------------------------------------------------------

CityTerrainLayerManager::CityTerrainLayerManager() :
	m_regions(),
	m_cityRegions(),
	m_cachedShaderTemplates(),
	m_cachedShaderNames(),
	m_shaderListDirty(true)
{
}

// ----------------------------------------------------------------------

CityTerrainLayerManager::~CityTerrainLayerManager()
{
	clearAllRegions();
}

// ----------------------------------------------------------------------

void CityTerrainLayerManager::addRegion(const TerrainRegion & region)
{
	removeRegion(region.regionId);

	TerrainRegion newRegion = region;
	newRegion.active = true;
	newRegion.timestamp = static_cast<int64>(time(0));
	newRegion.priority = ++s_regionPriorityCounter;
	loadShaderForRegion(newRegion);

	m_regions[region.regionId] = newRegion;
	m_cityRegions.insert(std::make_pair(region.cityId, region.regionId));

	REPORT_LOG(true, ("[Titan] CityTerrainLayerManager::addRegion: id=%s type=%d shader=%s cached=%p center=(%.1f,%.1f) radius=%.1f height=%.1f priority=%d\n",
		region.regionId.c_str(), static_cast<int>(newRegion.type), newRegion.shaderTemplate.c_str(),
		newRegion.cachedShader, newRegion.centerX, newRegion.centerZ, newRegion.radius, newRegion.height, newRegion.priority));

	// Invalidate the terrain chunks so they get rebuilt with the new shader/height
	// Add extra margin to ensure adjacent chunks are also rebuilt to prevent edge gaps
	TerrainObject * const terrainObject = TerrainObject::getInstance();
	if (terrainObject)
	{
		// Extra margin to account for chunk boundaries and ensure smooth transitions
		float const chunkMargin = 32.0f;  // Typical chunk size to ensure neighbors are invalidated
		float totalRadius = newRegion.radius + newRegion.blendDistance + chunkMargin;

		// For line regions, calculate bounding box
		if (newRegion.type == RT_SHADER_LINE)
		{
			float minX = (newRegion.centerX < newRegion.endX) ? newRegion.centerX : newRegion.endX;
			float maxX = (newRegion.centerX > newRegion.endX) ? newRegion.centerX : newRegion.endX;
			float minZ = (newRegion.centerZ < newRegion.endZ) ? newRegion.centerZ : newRegion.endZ;
			float maxZ = (newRegion.centerZ > newRegion.endZ) ? newRegion.centerZ : newRegion.endZ;
			float halfWidth = (newRegion.width * 0.5f) + newRegion.blendDistance + chunkMargin;

			Rectangle2d extent2d(
				minX - halfWidth,
				minZ - halfWidth,
				maxX + halfWidth,
				maxZ + halfWidth
			);
			terrainObject->invalidateRegion(extent2d);
		}
		else
		{
			// Circle region
			Rectangle2d extent2d(
				newRegion.centerX - totalRadius,
				newRegion.centerZ - totalRadius,
				newRegion.centerX + totalRadius,
				newRegion.centerZ + totalRadius
			);
			terrainObject->invalidateRegion(extent2d);
		}
		REPORT_LOG(true, ("[Titan] CityTerrainLayerManager::addRegion: invalidated terrain region with margin\n"));
	}

	// Notify any UI listeners that regions have changed
	if (ms_regionChangeCallback)
	{
		ms_regionChangeCallback(newRegion.cityId);
	}
}

// ----------------------------------------------------------------------

void CityTerrainLayerManager::removeRegion(const std::string & regionId)
{
	RegionMap::iterator it = m_regions.find(regionId);
	if (it != m_regions.end())
	{
		// Save region data for invalidation before removing
		TerrainRegion const removedRegion = it->second;
		int32 cityId = it->second.cityId;

		m_regions.erase(it);

		std::pair<CityRegionMap::iterator, CityRegionMap::iterator> range =
			m_cityRegions.equal_range(cityId);
		CityRegionMap::iterator cityIt = range.first;
		while (cityIt != range.second)
		{
			if (cityIt->second == regionId)
			{
				CityRegionMap::iterator toErase = cityIt;
				++cityIt;
				m_cityRegions.erase(toErase);
			}
			else
			{
				++cityIt;
			}
		}

		// Invalidate the terrain to restore original heights/shaders
		TerrainObject * const terrainObject = TerrainObject::getInstance();
		if (terrainObject)
		{
			float const chunkMargin = 32.0f;
			float totalRadius = removedRegion.radius + removedRegion.blendDistance + chunkMargin;

			if (removedRegion.type == RT_SHADER_LINE)
			{
				float minX = (removedRegion.centerX < removedRegion.endX) ? removedRegion.centerX : removedRegion.endX;
				float maxX = (removedRegion.centerX > removedRegion.endX) ? removedRegion.centerX : removedRegion.endX;
				float minZ = (removedRegion.centerZ < removedRegion.endZ) ? removedRegion.centerZ : removedRegion.endZ;
				float maxZ = (removedRegion.centerZ > removedRegion.endZ) ? removedRegion.centerZ : removedRegion.endZ;
				float halfWidth = (removedRegion.width * 0.5f) + removedRegion.blendDistance + chunkMargin;

				Rectangle2d extent2d(minX - halfWidth, minZ - halfWidth, maxX + halfWidth, maxZ + halfWidth);
				terrainObject->invalidateRegion(extent2d);
			}
			else
			{
				Rectangle2d extent2d(
					removedRegion.centerX - totalRadius,
					removedRegion.centerZ - totalRadius,
					removedRegion.centerX + totalRadius,
					removedRegion.centerZ + totalRadius
				);
				terrainObject->invalidateRegion(extent2d);
			}
			REPORT_LOG(true, ("[Titan] CityTerrainLayerManager::removeRegion: invalidated terrain for removed region %s\n", regionId.c_str()));
		}
	}
}

// ----------------------------------------------------------------------

void CityTerrainLayerManager::removeAllRegionsForCity(int32 cityId)
{
	std::pair<CityRegionMap::iterator, CityRegionMap::iterator> range =
		m_cityRegions.equal_range(cityId);

	std::vector<std::string> regionsToRemove;
	for (CityRegionMap::iterator it = range.first; it != range.second; ++it)
	{
		regionsToRemove.push_back(it->second);
	}

	for (size_t i = 0; i < regionsToRemove.size(); ++i)
	{
		m_regions.erase(regionsToRemove[i]);
	}

	m_cityRegions.erase(range.first, range.second);
}

// ----------------------------------------------------------------------

void CityTerrainLayerManager::clearAllRegions()
{
	m_regions.clear();
	m_cityRegions.clear();
}

// ----------------------------------------------------------------------

bool CityTerrainLayerManager::hasRegion(const std::string & regionId) const
{
	return m_regions.find(regionId) != m_regions.end();
}

// ----------------------------------------------------------------------

const CityTerrainLayerManager::TerrainRegion * CityTerrainLayerManager::getRegion(const std::string & regionId) const
{
	RegionMap::const_iterator it = m_regions.find(regionId);
	if (it != m_regions.end())
	{
		return &it->second;
	}
	return 0;
}

// ----------------------------------------------------------------------

void CityTerrainLayerManager::getRegionsAtLocation(float x, float z, std::vector<const TerrainRegion *> & outRegions) const
{
	outRegions.clear();

	for (RegionMap::const_iterator it = m_regions.begin(); it != m_regions.end(); ++it)
	{
		const TerrainRegion & region = it->second;
		if (!region.active)
			continue;

		bool inRegion = false;
		switch (region.type)
		{
		case RT_SHADER_CIRCLE:
		case RT_FLATTEN:
			inRegion = isPointInCircle(x, z, region.centerX, region.centerZ, region.radius + region.blendDistance);
			break;
		case RT_SHADER_LINE:
			inRegion = isPointInLine(x, z, region.centerX, region.centerZ, region.endX, region.endZ,
									 region.width + region.blendDistance);
			break;
		}

		if (inRegion)
		{
			outRegions.push_back(&region);
		}
	}
}

// ----------------------------------------------------------------------

void CityTerrainLayerManager::getRegionsForCity(int32 cityId, std::vector<const TerrainRegion *> & outRegions) const
{
	outRegions.clear();

	std::pair<CityRegionMap::const_iterator, CityRegionMap::const_iterator> range =
		m_cityRegions.equal_range(cityId);

	for (CityRegionMap::const_iterator it = range.first; it != range.second; ++it)
	{
		RegionMap::const_iterator regionIt = m_regions.find(it->second);
		if (regionIt != m_regions.end())
		{
			outRegions.push_back(&regionIt->second);
		}
	}
}

// ----------------------------------------------------------------------

void CityTerrainLayerManager::enumerateAvailableShaders(std::vector<std::string> & outTemplates, std::vector<std::string> & outNames) const
{
	if (m_shaderListDirty)
	{
		m_cachedShaderTemplates.clear();
		m_cachedShaderNames.clear();

		// Static list of ground/terrain shaders available for city painting
		// Using actual shader paths that exist in the game archives
		static const char * const shaderList[] = {
			"shader/floor_carpet_red.sht",
			"shader/floor_carpet_blue.sht",
			"shader/floor_carpet_green.sht",
			"shader/floor_carpet_brown.sht",
			"shader/floor_carpet_black.sht",
			"shader/floor_carpet_sand.sht",
			"shader/floor_carpet_purple.sht",
			"shader/floor_carpet_teal.sht",
			"shader/floor_carpet_orange.sht",
			"shader/floor_carpet_pink.sht",
			"shader/floor_all_tile_a.sht",
			"shader/floor_corellia_concrete_floor_a.sht",
			"shader/floor_corellia_marble_a_fallback.sht",
			"shader/all_imp_concrete_a_fallback_a2d13.sht",
			"shader/all_imp_concrete_b_fallback_a2d13.sht",
			"shader/sand_light.sht",
			"shader/grss_long_green.sht",
			"shader/dirt_forced_earth_cir1.sht",
			"shader/dirt_forced_earth_cir2.sht",
			"shader/dirt_forced_earth_cir3.sht",
			"shader/dark_vip_red_floor.sht",
			"shader/wter_lava.sht"
			"shader/wter_aqua.sht"
		};

		static const char * const nameList[] = {
			"Carpet (Red)",
			"Carpet (Blue)",
			"Carpet (Green)",
			"Carpet (Brown)",
			"Carpet (Black)",
			"Carpet (Sand)",
			"Carpet (Purple)",
			"Carpet (Teal)",
			"Carpet (Orange)",
			"Carpet (Pink)",
			"Floor Tile",
			"Concrete Floor (Corellia)",
			"Marble Floor (Corellia)",
			"Imperial Concrete A",
			"Imperial Concrete B",
			"Sand (Light)",
			"Grass (Long, Green)",
			"Dirt Earth 1",
			"Dirt Earth 2",
			"Dirt Earth 3",
			"VIP Floor (Red)",
			"Lava",
			"Water",
		};

		int const numShaders = sizeof(shaderList) / sizeof(shaderList[0]);

		for (int i = 0; i < numShaders; ++i)
		{
			if (TreeFile::exists(shaderList[i]))
			{
				m_cachedShaderTemplates.push_back(shaderList[i]);
				m_cachedShaderNames.push_back(nameList[i]);
			}
		}

		m_shaderListDirty = false;
	}

	outTemplates = m_cachedShaderTemplates;
	outNames = m_cachedShaderNames;
}

// ----------------------------------------------------------------------

bool CityTerrainLayerManager::getModifiedHeightInternal(float x, float z, float originalHeight, float & outHeight) const
{
	outHeight = originalHeight;
	bool modified = false;

	// Collect all flatten regions that affect this point, sorted by priority/timestamp
	struct RegionInfluence
	{
		const TerrainRegion * region;
		float weight;      // 0.0 = edge, 1.0 = full influence
		float distance;    // Distance from center
		int64 timestamp;
		int32 priority;
	};

	std::vector<RegionInfluence> influences;

	for (RegionMap::const_iterator it = m_regions.begin(); it != m_regions.end(); ++it)
	{
		const TerrainRegion & region = it->second;
		if (!region.active || region.type != RT_FLATTEN)
			continue;

		float totalRadius = region.radius + region.blendDistance;
		if (!isPointInCircle(x, z, region.centerX, region.centerZ, totalRadius))
			continue;

		float dist = static_cast<float>(sqrt(
			(x - region.centerX) * (x - region.centerX) +
			(z - region.centerZ) * (z - region.centerZ)));

		RegionInfluence inf;
		inf.region = &region;
		inf.distance = dist;
		inf.timestamp = region.timestamp;
		inf.priority = region.priority;

		if (dist <= region.radius)
		{
			inf.weight = 1.0f;
		}
		else
		{
			inf.weight = 1.0f - calculateBlendWeight(dist - region.radius, region.blendDistance);
		}

		influences.push_back(inf);
	}

	if (influences.empty())
		return false;

	// Sort by priority (higher first), then by timestamp (newer first)
	// This ensures newer/higher priority regions take precedence
	for (size_t i = 0; i < influences.size(); ++i)
	{
		for (size_t j = i + 1; j < influences.size(); ++j)
		{
			bool swap = false;
			if (influences[j].priority > influences[i].priority)
				swap = true;
			else if (influences[j].priority == influences[i].priority &&
					 influences[j].timestamp > influences[i].timestamp)
				swap = true;

			if (swap)
			{
				RegionInfluence temp = influences[i];
				influences[i] = influences[j];
				influences[j] = temp;
			}
		}
	}

	// Apply regions in order - the first one with full weight wins completely
	// For edge blending, we interpolate between the highest priority region and original terrain
	float finalHeight = originalHeight;
	float appliedWeight = 0.0f;

	for (size_t i = 0; i < influences.size(); ++i)
	{
		const RegionInfluence & inf = influences[i];

		if (inf.weight >= 0.999f)
		{
			// Full influence - this region completely determines the height
			finalHeight = inf.region->height;
			appliedWeight = 1.0f;
			modified = true;
			break;
		}
		else if (appliedWeight < 1.0f)
		{
			// Partial influence - blend with remaining weight
			float remainingWeight = 1.0f - appliedWeight;
			float contributionWeight = inf.weight * remainingWeight;

			if (contributionWeight > 0.001f)
			{
				// Blend between current height and this region's height
				finalHeight = finalHeight * (1.0f - contributionWeight) +
							  inf.region->height * contributionWeight;
				appliedWeight += contributionWeight;
				modified = true;
			}
		}
	}

	// Smooth transition: blend final result with original terrain at edges
	if (modified && appliedWeight < 1.0f)
	{
		float edgeBlend = appliedWeight;
		finalHeight = originalHeight * (1.0f - edgeBlend) + finalHeight * edgeBlend;
	}

	outHeight = finalHeight;
	return modified;
}

// ----------------------------------------------------------------------

bool CityTerrainLayerManager::getShaderOverrideInternal(float x, float z, Shader const *& outShader, float & outBlendWeight) const
{
	outShader = 0;
	outBlendWeight = 0.0f;

	// Periodic debug log
	static int s_callCount = 0;
	static bool s_reportedRegionCount = false;
	if (!s_reportedRegionCount && !m_regions.empty())
	{
		REPORT_LOG(true, ("[Titan] CityTerrainLayerManager::getShaderOverrideInternal: have %d regions\n", static_cast<int>(m_regions.size())));
		s_reportedRegionCount = true;
	}

	for (RegionMap::const_iterator it = m_regions.begin(); it != m_regions.end(); ++it)
	{
		const TerrainRegion & region = it->second;
		if (!region.active || !region.cachedShader)
			continue;

		if (region.type == RT_FLATTEN)
			continue;

		switch (region.type)
		{
		case RT_SHADER_CIRCLE:
			if (isPointInCircle(x, z, region.centerX, region.centerZ, region.radius + region.blendDistance))
			{
				float dist = static_cast<float>(sqrt(
					(x - region.centerX) * (x - region.centerX) +
					(z - region.centerZ) * (z - region.centerZ)));
				if (dist <= region.radius)
				{
					outShader = region.cachedShader;
					outBlendWeight = 1.0f;
				}
				else
				{
					float weight = calculateBlendWeight(dist - region.radius, region.blendDistance);
					if (weight > outBlendWeight)
					{
						outShader = region.cachedShader;
						outBlendWeight = weight;
					}
				}
			}
			break;

		case RT_SHADER_LINE:
			if (isPointInLine(x, z, region.centerX, region.centerZ, region.endX, region.endZ,
							  region.width + region.blendDistance))
			{
				float dx = region.endX - region.centerX;
				float dz = region.endZ - region.centerZ;
				float lineLen = static_cast<float>(sqrt(dx * dx + dz * dz));
				if (lineLen > 0.001f)
				{
					float perpDist = static_cast<float>(fabs((dz * (x - region.centerX) - dx * (z - region.centerZ)) / lineLen));
					float halfWidth = region.width * 0.5f;
					if (perpDist <= halfWidth)
					{
						outShader = region.cachedShader;
						outBlendWeight = 1.0f;
					}
					else if (perpDist <= halfWidth + region.blendDistance)
					{
						float weight = calculateBlendWeight(perpDist - halfWidth, region.blendDistance);
						if (weight > outBlendWeight)
						{
							outShader = region.cachedShader;
							outBlendWeight = weight;
						}
					}
				}
			}
			break;

		default:
			break;
		}
	}

	return outShader != 0;
}

// ----------------------------------------------------------------------

void CityTerrainLayerManager::update(float /*elapsedTime*/)
{
}

// ----------------------------------------------------------------------

void CityTerrainLayerManager::handleTerrainModifyMessage(
	int32 cityId,
	int32 modificationType,
	const std::string & regionId,
	const std::string & shaderTemplate,
	float centerX,
	float centerZ,
	float radius,
	float endX,
	float endZ,
	float width,
	float height,
	float blendDistance)
{
	REPORT_LOG(true, ("[Titan] CityTerrainLayerManager::handleTerrainModifyMessage: cityId=%d type=%d regionId=%s shader=%s center=(%.1f,%.1f) radius=%.1f height=%.1f\n",
		cityId, modificationType, regionId.c_str(), shaderTemplate.c_str(), centerX, centerZ, radius, height));

	if (modificationType == 3)
	{
		removeRegion(regionId);
		return;
	}

	if (modificationType == 4)
	{
		removeAllRegionsForCity(cityId);
		return;
	}

	TerrainRegion region;
	region.regionId = regionId;
	region.cityId = cityId;
	region.type = static_cast<RegionType>(modificationType);
	region.shaderTemplate = shaderTemplate;
	region.centerX = centerX;
	region.centerZ = centerZ;
	region.radius = radius;
	region.endX = endX;
	region.endZ = endZ;
	region.width = width;
	region.height = height;
	region.blendDistance = blendDistance;
	region.cachedShader = 0;
	region.active = true;
	region.timestamp = 0;  // Will be set by addRegion
	region.priority = 0;   // Will be set by addRegion

	addRegion(region);
}

// ----------------------------------------------------------------------

void CityTerrainLayerManager::handleTerrainSyncMessage(int32 /*cityId*/, const std::vector<TerrainRegion> & regions)
{
	for (size_t i = 0; i < regions.size(); ++i)
	{
		addRegion(regions[i]);
	}
}

// ----------------------------------------------------------------------

void CityTerrainLayerManager::flushTerrainUpdates()
{
	REPORT_LOG(true, ("[Titan] CityTerrainLayerManager::flushTerrainUpdates: flushing all %d regions\n",
		static_cast<int>(m_regions.size())));

	TerrainObject * const terrainObject = TerrainObject::getInstance();
	if (!terrainObject)
		return;

	// Invalidate all regions to force complete rebuild
	float const chunkMargin = 64.0f;  // Larger margin to ensure smooth blending across chunk boundaries

	for (RegionMap::const_iterator it = m_regions.begin(); it != m_regions.end(); ++it)
	{
		const TerrainRegion & region = it->second;

		if (region.type == RT_SHADER_LINE)
		{
			float minX = (region.centerX < region.endX) ? region.centerX : region.endX;
			float maxX = (region.centerX > region.endX) ? region.centerX : region.endX;
			float minZ = (region.centerZ < region.endZ) ? region.centerZ : region.endZ;
			float maxZ = (region.centerZ > region.endZ) ? region.centerZ : region.endZ;
			float halfWidth = (region.width * 0.5f) + region.blendDistance + chunkMargin;

			Rectangle2d extent2d(minX - halfWidth, minZ - halfWidth, maxX + halfWidth, maxZ + halfWidth);
			terrainObject->invalidateRegion(extent2d);
		}
		else
		{
			float totalRadius = region.radius + region.blendDistance + chunkMargin;
			Rectangle2d extent2d(
				region.centerX - totalRadius,
				region.centerZ - totalRadius,
				region.centerX + totalRadius,
				region.centerZ + totalRadius
			);
			terrainObject->invalidateRegion(extent2d);
		}
	}

	REPORT_LOG(true, ("[Titan] CityTerrainLayerManager::flushTerrainUpdates: completed\n"));
}

// ----------------------------------------------------------------------

void CityTerrainLayerManager::loadShaderForRegion(TerrainRegion & region)
{
	region.cachedShader = 0;

	if (!region.shaderTemplate.empty())
	{
		bool exists = TreeFile::exists(region.shaderTemplate.c_str());
		REPORT_LOG(true, ("[Titan] CityTerrainLayerManager::loadShaderForRegion: shader=%s exists=%d\n",
			region.shaderTemplate.c_str(), exists ? 1 : 0));

		if (exists)
		{
			region.cachedShader = ShaderTemplateList::fetchShader(region.shaderTemplate.c_str());
			REPORT_LOG(true, ("[Titan] CityTerrainLayerManager::loadShaderForRegion: loaded shader=%p\n", region.cachedShader));
		}
		else
		{
			// Try fallback shaders that are known to exist
			static char const * const fallbackShaders[] = {
				"shader/floor_carpet_red.sht",
				"shader/floor_all_tile_a.sht",
				"shader/all_imp_concrete_b_fallback_a2d13.sht"
			};
			static int const numFallbacks = sizeof(fallbackShaders) / sizeof(fallbackShaders[0]);

			for (int i = 0; i < numFallbacks && region.cachedShader == 0; ++i)
			{
				if (TreeFile::exists(fallbackShaders[i]))
				{
					region.cachedShader = ShaderTemplateList::fetchShader(fallbackShaders[i]);
					REPORT_LOG(true, ("[Titan] CityTerrainLayerManager::loadShaderForRegion: using fallback[%d]=%s shader=%p\n",
						i, fallbackShaders[i], region.cachedShader));
					break;
				}
			}
		}
	}
	else
	{
		REPORT_LOG(true, ("[Titan] CityTerrainLayerManager::loadShaderForRegion: empty shader template\n"));
	}
}

// ----------------------------------------------------------------------

bool CityTerrainLayerManager::isPointInCircle(float px, float pz, float cx, float cz, float radius) const
{
	float dx = px - cx;
	float dz = pz - cz;
	return (dx * dx + dz * dz) <= (radius * radius);
}

// ----------------------------------------------------------------------

bool CityTerrainLayerManager::isPointInLine(float px, float pz, float x1, float z1, float x2, float z2, float width) const
{
	float dx = x2 - x1;
	float dz = z2 - z1;
	float lineLenSq = dx * dx + dz * dz;

	if (lineLenSq < 0.0001f)
	{
		return isPointInCircle(px, pz, x1, z1, width * 0.5f);
	}

	float t = ((px - x1) * dx + (pz - z1) * dz) / lineLenSq;
	t = (t < 0.0f) ? 0.0f : ((t > 1.0f) ? 1.0f : t);

	float closestX = x1 + t * dx;
	float closestZ = z1 + t * dz;

	float distX = px - closestX;
	float distZ = pz - closestZ;
	float distSq = distX * distX + distZ * distZ;

	float halfWidth = width * 0.5f;
	return distSq <= (halfWidth * halfWidth);
}

// ----------------------------------------------------------------------

float CityTerrainLayerManager::calculateBlendWeight(float distance, float blendDistance) const
{
	if (blendDistance <= 0.0f)
		return 0.0f;

	float weight = 1.0f - (distance / blendDistance);
	return (weight < 0.0f) ? 0.0f : ((weight > 1.0f) ? 1.0f : weight);
}

// ======================================================================
