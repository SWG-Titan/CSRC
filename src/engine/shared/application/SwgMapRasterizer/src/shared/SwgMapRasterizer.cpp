// ======================================================================
//
// SwgMapRasterizer.cpp
// copyright 2026 
//
// Terrain map rasterization tool. Generates top-down planet map images
// from .trn terrain files for use in the UI (ui_planetmap.inc).
//
// Supports two modes:
//   - GPU Rendered Mode: Full client terrain rendering with shaders
//     and textures, captured via orthographic camera screenshots.
//   - CPU Colormap Mode: Uses the terrain generator to sample height
//     and color data, then applies hillshading for a 3D terrain map.
//
// ======================================================================

#include "FirstSwgMapRasterizer.h"
#include "SwgMapRasterizer.h"

// -- Engine shared includes --
#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedImage/SetupSharedImage.h"
#include "sharedImage/Image.h"
#include "sharedImage/TargaFormat.h"
#include "sharedMath/PackedRgb.h"
#include "sharedMath/SetupSharedMath.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/ObjectList.h"
#include "sharedObject/SetupSharedObject.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedRegex/SetupSharedRegex.h"
#include "sharedTerrain/ProceduralTerrainAppearanceTemplate.h"
#include "sharedTerrain/SetupSharedTerrain.h"
#include "sharedTerrain/TerrainGenerator.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedThread/SetupSharedThread.h"
#include "sharedUtility/SetupSharedUtility.h"

// -- Engine client includes (for GPU rendering mode) --
#include "clientGraphics/Camera.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/ConfigClientGraphics.h"
#include "clientGraphics/ScreenShotHelper.h"
#include "clientGraphics/SetupClientGraphics.h"
#include "clientObject/ObjectListCamera.h"
#include "clientObject/SetupClientObject.h"
#include "clientTerrain/SetupClientTerrain.h"

// -- Standard library --
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <direct.h>
#include <map>


// ======================================================================
// Static member initialization
// ======================================================================

bool      SwgMapRasterizer::s_engineInitialized    = false;
bool      SwgMapRasterizer::s_graphicsInitialized  = false;
HINSTANCE SwgMapRasterizer::s_hInstance             = NULL;


//
namespace
{
	std::map<std::string, PackedRgb> s_shaderFamilyColorCache;

	// ======================================================================
	// Helper: Read a single pixel from an image
	// ======================================================================
	static PackedRgb readPixelFromImage(const Image* image, int x, int y)
	{
		PackedRgb result = PackedRgb::solidBlack;

		if (!image || x < 0 || x >= image->getWidth() || y < 0 || y >= image->getHeight())
			return result;

		const uint8* data = image->lockReadOnly();
		if (!data)
			return result;

		// Move to the pixel location
		data += y * image->getStride() + x * image->getBytesPerPixel();

		// Handle different pixel formats
		if (image->getPixelFormat() == Image::PF_bgr_888)
		{
			result.b = *data++;
			result.g = *data++;
			result.r = *data++;
		}
		else if (image->getPixelFormat() == Image::PF_rgb_888)
		{
			result.r = *data++;
			result.g = *data++;
			result.b = *data++;
		}
		else
		{
			// For other formats, try to extract RGB components
			result.r = *data++;
			if (image->getBytesPerPixel() > 1)
				result.g = *data++;
			if (image->getBytesPerPixel() > 2)
				result.b = *data++;
		}

		image->unlock();

		return result;
	}

	// ======================================================================
	// Get representative color from a shader family's color ramp image
	// ======================================================================
	static PackedRgb getShaderFamilyColor(const char* familyName, const PackedRgb& fallback)
	{
		if (!familyName || !familyName[0])
			return fallback;

		// Check cache first
		auto it = s_shaderFamilyColorCache.find(familyName);
		if (it != s_shaderFamilyColorCache.end())
			return it->second;

	// Try to load the shader family's color ramp image
	// Following SOE convention: terrain/<familyname>_colorramp.tga
	char rampPath[256];
	_snprintf(rampPath, sizeof(rampPath), "terrain/%s_colorramp.tga", familyName);

	Image* image = NULL;
	TargaFormat tgaFormat;
	
	if (!tgaFormat.loadImage(rampPath, &image))
	{
		if (image)
			delete image;
		// Cache the fallback to avoid repeated failed loads
		s_shaderFamilyColorCache[familyName] = fallback;
		return fallback;
	}

		// Sample the middle of the ramp to get a representative color
		int sampleX = image->getWidth() / 2;
		int sampleY = image->getHeight() / 2;
		
		PackedRgb color = readPixelFromImage(image, sampleX, sampleY);

		delete image;

		// Cache the result
		s_shaderFamilyColorCache[familyName] = color;
		return color;
	}

	// ======================================================================
	// Blend two colors with a given weight
	// ======================================================================
	static PackedRgb blendColors(const PackedRgb& base, const PackedRgb& layer, float layerWeight)
	{
		float baseWeight = 1.0f - layerWeight;
		
		PackedRgb result;
		result.r = static_cast<uint8>(base.r * baseWeight + layer.r * layerWeight);
		result.g = static_cast<uint8>(base.g * baseWeight + layer.g * layerWeight);
		result.b = static_cast<uint8>(base.b * baseWeight + layer.b * layerWeight);
		
		return result;
	}

	// ======================================================================
	// Shader Layer: represents a single shader family and its properties
	// ======================================================================
	struct ShaderLayer
	{
		int familyId;
		std::string familyName;
		PackedRgb familyColor;
		int priority;
		float layerWeight;

		ShaderLayer() : familyId(0), priority(0), layerWeight(0.0f) {}
	};

	// ======================================================================
	// Apply shader layers to create a blended final color
	// Shader families with higher priority get more influence
	// ======================================================================
	static PackedRgb applyShaderLayers(
		const std::vector<ShaderLayer>& layers,
		const PackedRgb& baseColor,
		int pixelPriority
	)
	{
		if (layers.empty())
			return baseColor;

		PackedRgb result = baseColor;

		// Apply layers in priority order
		for (const auto& layer : layers)
		{
			// Skip layers with lower priority than the pixel
			if (layer.priority < pixelPriority)
				continue;

			// Weight factor based on family shader size and layer properties
			// Larger shader sizes create more prominent visual areas
			float weight = layer.layerWeight;
			
			result = blendColors(result, layer.familyColor, weight);
		}

		return result;
	}
}
// ======================================================================
// Planet table: terrain file -> output name mapping
// Maps .trn files to the UI resource names used in ui_planetmap.inc
// ======================================================================

const SwgMapRasterizer::PlanetEntry SwgMapRasterizer::s_planetEntries[] =
{
	{ "terrain/corellia.trn",             "ui_map_corellia"                       },
	{ "terrain/dantooine.trn",            "ui_map_dantooine"                      },
	{ "terrain/dathomir.trn",             "ui_map_dathomir"                       },
	{ "terrain/endor.trn",                "ui_map_endor"                          },
	{ "terrain/lok.trn",                  "ui_map_lok"                            },
	{ "terrain/naboo.trn",                "ui_map_naboo"                          },
	{ "terrain/rori.trn",                 "ui_map_rori"                           },
	{ "terrain/talus.trn",                "ui_map_talus"                          },
	{ "terrain/tatooine.trn",             "ui_map_tatooine"                       },
	{ "terrain/yavin.trn",                "ui_map_yavin4"                         },
	{ "terrain/taanab.trn",               "ui_map_taanab"                         },
	{ "terrain/chandrila.trn",            "ui_map_chandrila"                      },
	{ "terrain/hoth.trn",                 "ui_map_hoth"                           },
	{ "terrain/jakku.trn",                "ui_map_jakku"                          },


	/* Only full-gusto main planets for now, as the smaller sub-planet are not guaranteed to have their terrain files available in the TREs, and may require special handling for their smaller map sizes and different shader groups. Can add these back in later if needed.
	
	{"terrain/mustafar.trn",             "ui_map_mustafar"},
	{ "terrain/kashyyyk_main.trn",        "ui_map_kashyyyk_main"                  },
	{ "terrain/kashyyyk_dead_forest.trn", "ui_map_kashyyyk_dead_forest"           },
	{ "terrain/kashyyyk_hunting.trn",     "ui_map_kashyyyk_hunting"               },
	{ "terrain/kashyyyk_rryatt_trail.trn","ui_map_kashyyyk_rryatt_trail"          },
	{ "terrain/kashyyyk_north_dungeons.trn","ui_map_kashyyyk_north_dungeons_slaver"},
	{ "terrain/kashyyyk_south_dungeons.trn","ui_map_kashyyyk_south_dungeons_bocctyyy"},*/
};

const int SwgMapRasterizer::s_numPlanetEntries = sizeof(s_planetEntries) / sizeof(s_planetEntries[0]);
static PackedRgb getShaderFamilyColor(
	const ShaderGroup& shaderGroup,
	const ShaderGroup::Info& sgi,
	const PackedRgb& fallback
);
// ======================================================================
// Config defaults
// ======================================================================

SwgMapRasterizer::Config::Config() :
	terrainFile(""),
	outputDir("./maps"),
	imageSize(1024),
	tilesPerSide(4),
	cameraHeight(20000.0f),
	useColormapMode(true),
	processAll(false),
	configFile("titan.cfg")
{
}

// ======================================================================
// Banner and usage
// ======================================================================

void SwgMapRasterizer::printBanner()
{
	printf("========================================================\n");
	printf("  SwgMapRasterizer - Planet Map Image Generator\n");
	printf("  Generates top-down terrain map images for the UI\n");
	printf("========================================================\n\n");
}

// ----------------------------------------------------------------------

void SwgMapRasterizer::printUsage()
{
	printf("Usage: SwgMapRasterizer.exe [options]\n\n");
	printf("Options:\n");
	printf("  -terrain <file.trn>   Terrain file to rasterize\n");
	printf("  -output <dir>         Output directory (default: ./maps)\n");
	printf("  -size <pixels>        Output image size, square (default: 1024)\n");
	printf("  -tiles <n>            Tile grid divisions per side (default: 4)\n");
	printf("  -height <meters>      Camera height for GPU mode (default: 20000)\n");
	printf("  -colormap             Use CPU colormap mode (default)\n");
	printf("  -render               Use GPU rendered mode\n");
	printf("  -all                  Process all known planet terrains\n");
	printf("  -config <file.cfg>    Engine config file (default: client.cfg)\n");
	printf("  -help                 Show this help message\n");
	printf("\n");
	printf("\nExamples:\n");
	printf("  SwgMapRasterizer.exe -terrain terrain/tatooine.trn\n");
	printf("  SwgMapRasterizer.exe -all -size 2048\n");
	printf("  SwgMapRasterizer.exe -terrain terrain/naboo.trn -render\n");
	printf("\nKnown planets:\n");
	for (int i = 0; i < s_numPlanetEntries; ++i)
	{
		printf("  %-40s -> %s\n", s_planetEntries[i].terrainFile, s_planetEntries[i].outputName);
	}
	printf("Once generated, make a new .dds with dxtex.exe and update ui_planetmap.inc with the new filename and dimensions.\n");
	printf("Automation: Disabled");
	exit(0);
}

// ======================================================================
// Command line parsing
// ======================================================================

SwgMapRasterizer::Config SwgMapRasterizer::parseCommandLine(const char* commandLine)
{
	Config config;

	if (!commandLine || !*commandLine)
		return config;

	// Tokenize the command line
	std::vector<std::string> args;
	const char* p = commandLine;
	while (*p)
	{
		// Skip whitespace
		while (*p == ' ' || *p == '\t') ++p;
		if (!*p) break;

		// Handle quoted strings
		if (*p == '"')
		{
			++p;
			const char* start = p;
			while (*p && *p != '"') ++p;
			args.push_back(std::string(start, p));
			if (*p == '"') ++p;
		}
		else
		{
			const char* start = p;
			while (*p && *p != ' ' && *p != '\t') ++p;
			args.push_back(std::string(start, p));
		}
	}

	// Parse tokens
	for (size_t i = 0; i < args.size(); ++i)
	{
		const std::string& arg = args[i];

		if ((arg == "-terrain" || arg == "-t") && i + 1 < args.size())
		{
			config.terrainFile = args[++i];
		}
		else if ((arg == "-output" || arg == "-o") && i + 1 < args.size())
		{
			config.outputDir = args[++i];
		}
		else if ((arg == "-size" || arg == "-s") && i + 1 < args.size())
		{
			config.imageSize = atoi(args[++i].c_str());
			if (config.imageSize < 64) config.imageSize = 64;
			if (config.imageSize > 8192) config.imageSize = 8192;
		}
		else if ((arg == "-tiles") && i + 1 < args.size())
		{
			config.tilesPerSide = atoi(args[++i].c_str());
			if (config.tilesPerSide < 1) config.tilesPerSide = 1;
			if (config.tilesPerSide > 64) config.tilesPerSide = 64;
		}
		else if ((arg == "-height") && i + 1 < args.size())
		{
			config.cameraHeight = static_cast<float>(atof(args[++i].c_str()));
		}
		else if (arg == "-colormap")
		{
			config.useColormapMode = true;
		}
		else if (arg == "-render")
		{
			config.useColormapMode = false;
		}
		else if (arg == "-all")
		{
			config.processAll = true;
		}
		else if ((arg == "-config") && i + 1 < args.size())
		{
			config.configFile = args[++i];
		}
		else if (arg == "-help" || arg == "-h" || arg == "--help")
		{
			printUsage();
		}
	}

	return config;
}

// ======================================================================
// Engine initialization
// ======================================================================

bool SwgMapRasterizer::initializeEngine(HINSTANCE hInstance, const Config& config)
{
	s_hInstance = hInstance;

	// -- Thread --
	SetupSharedThread::install();

	// -- Debug --
	SetupSharedDebug::install(4096);

	// -- Foundation --
	if (config.useColormapMode)
	{
		// CPU colormap mode: console application, no window needed
		SetupSharedFoundation::Data data(SetupSharedFoundation::Data::D_console);
		data.configFile = config.configFile.c_str();
		SetupSharedFoundation::install(data);
	}
	else
	{
		// GPU render mode: game mode for window/D3D
		SetupSharedFoundation::Data data(SetupSharedFoundation::Data::D_game);
		data.windowName    = "SwgMapRasterizer";
		data.hInstance     = hInstance;
		data.configFile    = config.configFile.c_str();
		data.clockUsesSleep = true;
		data.frameRateLimit = 60.0f;
		SetupSharedFoundation::install(data);
	}

	if (ConfigFile::isEmpty())
	{
		printf("ERROR: Config file '%s' not found or empty.\n", config.configFile.c_str());
		printf("       The config file must contain [SharedFile] entries for TRE paths.\n");
		return false;
	}

	// -- Compression --
	{
		SetupSharedCompression::Data data;
		data.numberOfThreadsAccessingZlib = 1;
		SetupSharedCompression::install(data);
	}

	// -- Regex --
	SetupSharedRegex::install();

	// -- File system (with tree files for .trn access) --
	SetupSharedFile::install(true);

	// -- Math --
	SetupSharedMath::install();

	// -- Utility --
	{
		SetupSharedUtility::Data data;
		SetupSharedUtility::setupGameData(data);
		SetupSharedUtility::install(data);
	}

	// -- Random --
	SetupSharedRandom::install(static_cast<uint32>(time(NULL)));

	// -- Image --
	SetupSharedImage::Data data;
	SetupSharedImage::setupDefaultData(data);
	SetupSharedImage::install(data);

	// -- Object --
	{
		SetupSharedObject::Data data;
		SetupSharedObject::setupDefaultGameData(data);
		SetupSharedObject::install(data);
	}

	// -- Shared Terrain --
	{
		SetupSharedTerrain::Data data;
		SetupSharedTerrain::setupGameData(data);
		SetupSharedTerrain::install(data);
	}

	s_engineInitialized = true;

	// -- Client-side subsystems for GPU rendering mode --
	if (!config.useColormapMode)
	{
		// Graphics
		SetupClientGraphics::Data graphicsData;
		graphicsData.screenWidth  = config.imageSize;
		graphicsData.screenHeight = config.imageSize;
		graphicsData.alphaBufferBitDepth = 0;
		SetupClientGraphics::setupDefaultGameData(graphicsData);

		if (!SetupClientGraphics::install(graphicsData))
		{
			printf("WARNING: Failed to initialize graphics. Falling back to colormap mode.\n");
			return true; // Engine is initialized, just no graphics
		}

		s_graphicsInitialized = true;

		// Client Object (for ObjectListCamera)
		{
			SetupClientObject::Data data;
			SetupClientObject::setupGameData(data);
			SetupClientObject::install(data);
		}

		// Client Terrain (for rendered terrain appearance)
		SetupClientTerrain::install();
	}

	return true;
}

// ----------------------------------------------------------------------

void SwgMapRasterizer::shutdownEngine()
{
	if (s_engineInitialized)
	{
		SetupSharedFoundation::remove();
		SetupSharedThread::remove();
		s_engineInitialized = false;
		s_graphicsInitialized = false;
	}
}

// ======================================================================
// Main entry point
// ======================================================================

int SwgMapRasterizer::run(HINSTANCE hInstance, const char* commandLine)
{
	printBanner();

	Config config = parseCommandLine(commandLine);
	
	//if config is empty, show usage
	if (commandLine == nullptr || strlen(commandLine) == 0)
	{
		printUsage();
		return 1;
	}

	// Validate input
	if (!config.processAll && config.terrainFile.empty())
	{
		printf("ERROR: No terrain file specified. Use -terrain <file.trn> or -all.\n\n");
		printUsage();
		return 1;
	}

	// Ensure image size is divisible by tiles
	if (config.imageSize % config.tilesPerSide != 0)
	{
		config.imageSize = (config.imageSize / config.tilesPerSide) * config.tilesPerSide;
		printf("NOTE: Adjusted image size to %d to be divisible by tile count.\n", config.imageSize);
	}

	printf("Configuration:\n");
	printf("  Mode:       %s\n", config.useColormapMode ? "CPU Colormap" : "GPU Rendered");
	printf("  Image size: %dx%d\n", config.imageSize, config.imageSize);
	printf("  Tile grid:  %dx%d (%d tiles)\n", config.tilesPerSide, config.tilesPerSide, 
	       config.tilesPerSide * config.tilesPerSide);
	printf("  Output dir: %s\n", config.outputDir.c_str());
	printf("  Config:     %s\n", config.configFile.c_str());
	printf("\n");

	// Initialize engine
	printf("Initializing engine...\n");
	if (!initializeEngine(hInstance, config))
	{
		printf("FATAL: Engine initialization failed.\n");
		shutdownEngine();
		return 1;
	}
	printf("Engine initialized successfully.\n\n");

	// Create output directory
	_mkdir(config.outputDir.c_str());

	int successCount = 0;
	int failCount = 0;

	if (config.processAll)
	{
		// Process all known planet terrains
		printf("Processing all %d planet terrains...\n\n", s_numPlanetEntries);
		for (int i = 0; i < s_numPlanetEntries; ++i)
		{
			printf("--- [%d/%d] %s ---\n", i + 1, s_numPlanetEntries, s_planetEntries[i].terrainFile);
			if (rasterizeTerrain(config, s_planetEntries[i].terrainFile, s_planetEntries[i].outputName))
				++successCount;
			else
				++failCount;
			printf("\n");
		}
	}
	else
	{
		// Process single terrain file
		// Determine output name from terrain file
		std::string outputName;

		// Check if it matches a known planet
		for (int i = 0; i < s_numPlanetEntries; ++i)
		{
			if (config.terrainFile == s_planetEntries[i].terrainFile)
			{
				outputName = s_planetEntries[i].outputName;
				break;
			}
		}

		// Generate output name from terrain filename if not found
		if (outputName.empty())
		{
			std::string name = config.terrainFile;
			// Strip path
			size_t lastSlash = name.find_last_of("/\\");
			if (lastSlash != std::string::npos)
				name = name.substr(lastSlash + 1);
			// Strip extension
			size_t dot = name.find_last_of('.');
			if (dot != std::string::npos)
				name = name.substr(0, dot);
			outputName = "ui_map_" + name;
		}

		printf("Processing: %s -> %s\n\n", config.terrainFile.c_str(), outputName.c_str());
		if (rasterizeTerrain(config, config.terrainFile.c_str(), outputName.c_str()))
			++successCount;
		else
			++failCount;
	}

	printf("\n========================================================\n");
	printf("  Complete: %d succeeded, %d failed\n", successCount, failCount);
	printf("========================================================\n");

	shutdownEngine();
	exit(failCount > 0 ? 1 : 0);
}

// ======================================================================
// Terrain processing dispatcher
// ======================================================================
bool SwgMapRasterizer::rasterizeTerrain(const Config& config, const char* terrainFile, const char* outputName)
{
	// Check if terrain file exists in the tree file system
	if (!TreeFile::exists(terrainFile))
	{
		printf("  ERROR: Terrain file '%s' not found in tree file system.\n", terrainFile);
		printf("         Make sure the TRE files are correctly configured in %s.\n", config.configFile.c_str());
		return false;
	}

	if (config.useColormapMode || !s_graphicsInitialized)
	{
		return renderTerrainColormap(config, terrainFile, outputName);
	}
	else
	{
		return renderTerrainGPU(config, terrainFile, outputName);
	}
}

// ======================================================================
// CPU Colormap Mode
// ======================================================================

// ======================================================================
// Build shader layer information from the terrain generator
// ======================================================================
static std::vector<ShaderLayer> buildShaderLayers(const TerrainGenerator* generator)
{
	std::vector<ShaderLayer> layers;

	if (!generator)
		return layers;

	const ShaderGroup& shaderGroup = generator->getShaderGroup();
	int numFamilies = shaderGroup.getNumberOfFamilies();

	printf("  Found %d shader families:\n", numFamilies);

	// Enumerate all shader families in the generator
	for (int familyIndex = 0; familyIndex < numFamilies; ++familyIndex)
	{
		int familyId = shaderGroup.getFamilyId(familyIndex);
		const char* familyName = shaderGroup.getFamilyName(familyId);
		const PackedRgb& familyColor = shaderGroup.getFamilyColor(familyId);
		float shaderSize = shaderGroup.getFamilyShaderSize(familyId);
		int numChildren = shaderGroup.getFamilyNumberOfChildren(familyId);

		printf("    [%d] %s (id=%d, color=%d,%d,%d, size=%.1f m, children=%d)\n",
			familyIndex, familyName, familyId, 
			familyColor.r, familyColor.g, familyColor.b,
			shaderSize, numChildren);

		ShaderLayer layer;
		layer.familyId = familyId;
		layer.familyName = familyName ? familyName : "";
		layer.familyColor = familyColor;
		layer.priority = familyIndex; // Priority is based on order in family list
		layer.layerWeight = 0.3f;     // Default blend weight for shader color influence

		// Adjust weight based on shader size for visual prominence
		// Larger shader sizes should have more visual impact
		if (shaderSize > 100.0f)
			layer.layerWeight = 0.4f;
		else if (shaderSize < 10.0f)
			layer.layerWeight = 0.2f;

		layers.push_back(layer);
	}

	printf("  Shader family enumeration complete.\n\n");

	return layers;
}

bool SwgMapRasterizer::renderTerrainColormap(const Config& config, const char* terrainFile, const char* outputName)
{
	printf("  Loading terrain: %s\n", terrainFile);

	// Load the terrain template through the appearance system
	const AppearanceTemplate* at = AppearanceTemplateList::fetch(terrainFile);
	if (!at)
	{
		printf("  ERROR: Failed to load terrain template '%s'.\n", terrainFile);
		return false;
	}

	const ProceduralTerrainAppearanceTemplate* terrainTemplate = 
		dynamic_cast<const ProceduralTerrainAppearanceTemplate*>(at);

	if (!terrainTemplate)
	{
		printf("  ERROR: '%s' is not a procedural terrain file.\n", terrainFile);
		AppearanceTemplateList::release(at);
		return false;
	}

	// Get terrain properties
	const float mapWidth  = terrainTemplate->getMapWidthInMeters();
	const bool legacyMode = terrainTemplate->getLegacyMode();
	const int originOffset = terrainTemplate->getChunkOriginOffset();
	const int upperPad     = terrainTemplate->getChunkUpperPad();

	const TerrainGenerator* generator = terrainTemplate->getTerrainGenerator();
	const ShaderGroup& shaderGroup = generator->getShaderGroup();
	if (!generator)
	{
		printf("  ERROR: Terrain has no generator.\n");
		AppearanceTemplateList::release(at);
		return false;
	}

	printf("  Map width: %.0f meters (%.0f x %.0f km)\n", mapWidth, mapWidth / 1000.0f, mapWidth / 1000.0f);
	printf("  Legacy mode: %s\n", legacyMode ? "yes" : "no");
	printf("  Chunk padding: origin=%d, upper=%d\n", originOffset, upperPad);

	// Build shader layer information from the generator
	printf("\n  Building shader layer information...\n");
	std::vector<ShaderLayer> shaderLayers = buildShaderLayers(generator);
	printf("\n");

	const int imageSize = config.imageSize;
	const int tilesPerSide = config.tilesPerSide;
	const int pixelsPerTile = imageSize / tilesPerSide;
	const float metersPerPixel = mapWidth / static_cast<float>(imageSize);

	printf("  Output: %dx%d pixels (%.2f meters/pixel)\n", imageSize, imageSize, metersPerPixel);
	printf("  Generating terrain data in %dx%d tiles...\n", tilesPerSide, tilesPerSide);

	// Allocate output buffers
	uint8* colorPixels = new uint8[imageSize * imageSize * 3];
	float* heightPixels = new float[imageSize * imageSize];
	memset(colorPixels, 0, imageSize * imageSize * 3);
	memset(heightPixels, 0, imageSize * imageSize * sizeof(float));

	float minHeight =  1e10f;
	float maxHeight = -1e10f;

	// Process each tile
	int totalTiles = tilesPerSide * tilesPerSide;
	int tilesDone = 0;

	for (int tz = 0; tz < tilesPerSide; ++tz)
	{
		for (int tx = 0; tx < tilesPerSide; ++tx)
		{
			++tilesDone;
			printf("\r  Generating Tile [%d/%d] (%d,%d)...", tilesDone, totalTiles, tx, tz);
			fflush(stdout);

			// World-space origin of this tile
			const float tileWorldMinX = -mapWidth / 2.0f + tx * (mapWidth / tilesPerSide);
			const float tileWorldMinZ = -mapWidth / 2.0f + tz * (mapWidth / tilesPerSide);

			// Set up chunk generation request
			const int totalPoles = pixelsPerTile + originOffset + upperPad;

			TerrainGenerator::CreateChunkBuffer* buffer =
				new TerrainGenerator::CreateChunkBuffer();
			memset(buffer, 0, sizeof(TerrainGenerator::CreateChunkBuffer));
			buffer->allocate(totalPoles);

			TerrainGenerator::GeneratorChunkData chunkData(legacyMode);
			chunkData.originOffset        = originOffset;
			chunkData.numberOfPoles       = totalPoles;
			chunkData.upperPad            = upperPad;
			chunkData.distanceBetweenPoles = metersPerPixel;
			chunkData.start               = Vector(
				tileWorldMinX - originOffset * metersPerPixel,
				0.0f,
				tileWorldMinZ - originOffset * metersPerPixel
			);

			// Connect map pointers
			chunkData.heightMap                    = &buffer->heightMap;
			chunkData.colorMap                     = &buffer->colorMap;
			chunkData.shaderMap                    = &buffer->shaderMap;
			chunkData.floraStaticCollidableMap      = &buffer->floraStaticCollidableMap;
			chunkData.floraStaticNonCollidableMap   = &buffer->floraStaticNonCollidableMap;
			chunkData.floraDynamicNearMap           = &buffer->floraDynamicNearMap;
			chunkData.floraDynamicFarMap            = &buffer->floraDynamicFarMap;
			chunkData.environmentMap                = &buffer->environmentMap;
			chunkData.vertexPositionMap             = &buffer->vertexPositionMap;
			chunkData.vertexNormalMap               = &buffer->vertexNormalMap;
			chunkData.excludeMap                    = &buffer->excludeMap;
			chunkData.passableMap                   = &buffer->passableMap;

			// Set group pointers from the generator
			chunkData.shaderGroup       = &generator->getShaderGroup();
			chunkData.floraGroup        = &generator->getFloraGroup();
			chunkData.radialGroup       = &generator->getRadialGroup();
			chunkData.environmentGroup  = &generator->getEnvironmentGroup();
			chunkData.fractalGroup      = &generator->getFractalGroup();
			chunkData.bitmapGroup       = &generator->getBitmapGroup();

			// Generate terrain chunk data
			generator->generateChunk(chunkData);
			

			// Copy height and color data to output image
			for (int z = 0; z < pixelsPerTile; ++z)
			{
				for (int x = 0; x < pixelsPerTile; ++x)
				{
					const int srcX = x + originOffset;
					const int srcZ = z + originOffset;

					// Destination pixel in the full image
					const int dstX = tx * pixelsPerTile + x;
					const int dstZ = tz * pixelsPerTile + z;

					// Image Y is inverted: top of image = north = max Z
					const int imgX = dstX;
					const int imgY = (imageSize - 1) - dstZ;

					if (imgX < 0 || imgX >= imageSize || imgY < 0 || imgY >= imageSize)
						continue;

					// Get base color from generator (includes layered colors from affectors)
					const PackedRgb generatorColor =
						buffer->colorMap.getData(srcX, srcZ);

					// Get shader family info for this pixel
					const ShaderGroup::Info sgi =
						buffer->shaderMap.getData(srcX, srcZ);

					// Start with generator color as base
					PackedRgb color = generatorColor;

					// Apply shader layers for enhanced visual representation
					if (sgi.getFamilyId() > 0)
					{
						int pixelPriority = sgi.getPriority();
						
						// Get the shader family name
						const char* familyName = chunkData.shaderGroup->getFamilyName(sgi.getFamilyId());
						if (familyName && familyName[0])
						{
							// Get the representative color for this shader family
							PackedRgb shaderColor = getShaderFamilyColor(familyName, generatorColor);

							// Apply shader layers blending
							color = applyShaderLayers(shaderLayers, generatorColor, pixelPriority);

							// If no layers matched, blend shader color directly
							if (color.r == generatorColor.r && color.g == generatorColor.g && color.b == generatorColor.b)
							{
								// Blend with shader family color (40% shader, 60% generator)
								color = blendColors(generatorColor, shaderColor, 0.4f);
							}
						}
					}

					const float height =
						buffer->heightMap.getData(srcX, srcZ);

					const int pixelIndex = imgY * imageSize + imgX;
					const int colorOffset = pixelIndex * 3;

					colorPixels[colorOffset + 0] = color.r;
					colorPixels[colorOffset + 1] = color.g;
					colorPixels[colorOffset + 2] = color.b;

					heightPixels[pixelIndex] = height;

					if (height < minHeight) minHeight = height;
					if (height > maxHeight) maxHeight = height;
				}
			}
			delete buffer;
		}
	}

	printf("\r  Generated %d tiles. Height range: %.1f to %.1f meters\n", totalTiles, minHeight, maxHeight);

	// Apply hillshading for 3D terrain effect
	printf("  Applying hillshading...\n");
	applyHillshading(heightPixels, colorPixels, imageSize, imageSize, mapWidth);

	// Apply water rendering
	if (terrainTemplate->getUseGlobalWaterTable())
	{
		const float waterHeight = terrainTemplate->getGlobalWaterTableHeight();
		printf("  Rendering water (global water table at %.1f meters)...\n", waterHeight);

		for (int y = 0; y < imageSize; ++y)
		{
			for (int x = 0; x < imageSize; ++x)
			{
				const int idx = y * imageSize + x;
				const float h = heightPixels[idx];

				if (h <= waterHeight)
				{
					const int offset = idx * 3;
					const float depth = waterHeight - h;
					const float waterFactor = std::min(depth / 30.0f, 0.85f);

					// Water color: deep blue, default until shading implementation is added
					const float waterR = 15.0f;
					const float waterG = 50.0f;
					const float waterB = 110.0f;

					// Acid color: deep blue, default until shading implementation is added
					const float acidR = 60.0f;
					const float acidG = 120.0f;
					const float acidB = 30.0f;

					//Lava color - bright orange, default until shading implementation is added
					const float lavaR = 200.0f;
					const float lavaG = 80.0f;
					const float lavaB = 20.0f;

					//Using TerrainGeneratorWaterType to determine if we should render water, acid, or lava based on the terrain's water type
					
					//init vector with water color by default, then override it if it's acid or lava

					Vector waterColor(waterR, waterG, waterB);
					Vector w_postion(x, waterHeight, y);

					const TerrainGeneratorWaterType waterType = terrainTemplate->getWaterType(w_postion);
					if (waterType == TerrainGeneratorWaterType::TGWT_lava)
					{
						waterColor = Vector(lavaR, lavaG, lavaB);
					}
					else if (waterType == TerrainGeneratorWaterType::TGWT_water)
					{
						waterColor = Vector(waterR, waterG, waterB);
					}

					colorPixels[offset + 0] = static_cast<uint8>(colorPixels[offset + 0] * (1.0f - waterFactor) + waterColor.x * waterFactor);
					colorPixels[offset + 1] = static_cast<uint8>(colorPixels[offset + 1] * (1.0f - waterFactor) + waterColor.y * waterFactor);
					colorPixels[offset + 2] = static_cast<uint8>(colorPixels[offset + 2] * (1.0f - waterFactor) + waterColor.z * waterFactor);
				}
			}
		}
	}

	//flip vertically to match the expected orientation in the UI
	const int rowBytes = imageSize * 3;
	std::vector<uint8> flippedPixels(imageSize* imageSize * 3);
	for (int y = 0; y < imageSize; ++y)
	{
		const int srcOffset = y * rowBytes;
		const int dstOffset = (imageSize - 1 - y) * rowBytes;
		memcpy(&flippedPixels[dstOffset], &colorPixels[srcOffset], rowBytes);
	}
	printf("  Flipped image vertically for correct orientation.\n");

	// Save output image
	char outputPath[512];
	_snprintf(outputPath, sizeof(outputPath), "%s/%s.tga", config.outputDir.c_str(), outputName);

	printf("  Saving: %s\n", outputPath);
	const bool saved = saveTGA(colorPixels, imageSize, imageSize, 3, outputPath);

	// Cleanup
	delete[] colorPixels;
	delete[] heightPixels;
	AppearanceTemplateList::release(at);

	if (saved)
		printf("  SUCCESS: %s (%dx%d)\n", outputPath, imageSize, imageSize);
	else
		printf("  ERROR: Failed to save %s\n", outputPath);

	return saved;
}

bool SwgMapRasterizer::renderTerrainShading(const Config& config, const char* terrainFile, const char* outputName)
{
	printf("  Loading terrain: %s\n", terrainFile);

	// Load the terrain template through the appearance system
	const AppearanceTemplate* at = AppearanceTemplateList::fetch(terrainFile);
	if (!at)
	{
		printf("  ERROR: Failed to load terrain template '%s'.\n", terrainFile);
		return false;
	}

	const ProceduralTerrainAppearanceTemplate* terrainTemplate =
		dynamic_cast<const ProceduralTerrainAppearanceTemplate*>(at);

	if (!terrainTemplate)
	{
		printf("  ERROR: '%s' is not a procedural terrain file.\n", terrainFile);
		AppearanceTemplateList::release(at);
		return false;
	}

	// Get terrain properties
	const float mapWidth = terrainTemplate->getMapWidthInMeters();
	const bool legacyMode = terrainTemplate->getLegacyMode();
	const int originOffset = terrainTemplate->getChunkOriginOffset();
	const int upperPad = terrainTemplate->getChunkUpperPad();

	const TerrainGenerator* generator = terrainTemplate->getTerrainGenerator();
	if (!generator)
	{
		printf("  ERROR: Terrain has no generator.\n");
		AppearanceTemplateList::release(at);
		return false;
	}

	printf("  Map width: %.0f meters (%.0f x %.0f km)\n", mapWidth, mapWidth / 1000.0f, mapWidth / 1000.0f);
	printf("  Legacy mode: %s\n", legacyMode ? "yes" : "no");
	printf("  Chunk padding: origin=%d, upper=%d\n", originOffset, upperPad);

	const int imageSize = config.imageSize;
	const int tilesPerSide = config.tilesPerSide;
	const int pixelsPerTile = imageSize / tilesPerSide;
	const float metersPerPixel = mapWidth / static_cast<float>(imageSize);

	printf("  Output: %dx%d pixels (%.2f meters/pixel)\n", imageSize, imageSize, metersPerPixel);
	printf("  Generating terrain data in %dx%d tiles...\n", tilesPerSide, tilesPerSide);

	// Allocate output buffers
	uint8* colorPixels = new uint8[imageSize * imageSize * 3];
	float* heightPixels = new float[imageSize * imageSize];
	memset(colorPixels, 0, imageSize * imageSize * 3);
	memset(heightPixels, 0, imageSize * imageSize * sizeof(float));

	float minHeight = 1e10f;
	float maxHeight = -1e10f;

	// Process each tile
	int totalTiles = tilesPerSide * tilesPerSide;
	int tilesDone = 0;

	for (int tz = 0; tz < tilesPerSide; ++tz)
	{
		for (int tx = 0; tx < tilesPerSide; ++tx)
		{
			++tilesDone;
			printf("\r  Generating Tile [%d/%d] (%d,%d)...", tilesDone, totalTiles, tx, tz);
			fflush(stdout);

			// World-space origin of this tile
			const float tileWorldMinX = -mapWidth / 2.0f + tx * (mapWidth / tilesPerSide);
			const float tileWorldMinZ = -mapWidth / 2.0f + tz * (mapWidth / tilesPerSide);

			// Set up chunk generation request
			const int totalPoles = pixelsPerTile + originOffset + upperPad;

			TerrainGenerator::CreateChunkBuffer* buffer =
				new TerrainGenerator::CreateChunkBuffer();
			memset(buffer, 0, sizeof(TerrainGenerator::CreateChunkBuffer));
			buffer->allocate(totalPoles);

			TerrainGenerator::GeneratorChunkData chunkData(legacyMode);
			chunkData.originOffset = originOffset;
			chunkData.numberOfPoles = totalPoles;
			chunkData.upperPad = upperPad;
			chunkData.distanceBetweenPoles = metersPerPixel;
			chunkData.start = Vector(
				tileWorldMinX - originOffset * metersPerPixel,
				0.0f,
				tileWorldMinZ - originOffset * metersPerPixel
			);

			// Connect map pointers
			chunkData.heightMap = &buffer->heightMap;
			chunkData.colorMap = &buffer->colorMap;
			chunkData.shaderMap = &buffer->shaderMap;
			chunkData.floraStaticCollidableMap = &buffer->floraStaticCollidableMap;
			chunkData.floraStaticNonCollidableMap = &buffer->floraStaticNonCollidableMap;
			chunkData.floraDynamicNearMap = &buffer->floraDynamicNearMap;
			chunkData.floraDynamicFarMap = &buffer->floraDynamicFarMap;
			chunkData.environmentMap = &buffer->environmentMap;
			chunkData.vertexPositionMap = &buffer->vertexPositionMap;
			chunkData.vertexNormalMap = &buffer->vertexNormalMap;
			chunkData.excludeMap = &buffer->excludeMap;
			chunkData.passableMap = &buffer->passableMap;

			// Set group pointers from the generator
			chunkData.shaderGroup = &generator->getShaderGroup();
			chunkData.floraGroup = &generator->getFloraGroup();
			chunkData.radialGroup = &generator->getRadialGroup();
			chunkData.environmentGroup = &generator->getEnvironmentGroup();
			chunkData.fractalGroup = &generator->getFractalGroup();
			chunkData.bitmapGroup = &generator->getBitmapGroup();

			// Generate terrain chunk data
			generator->generateChunk(chunkData);

			// Copy height and color data to output image
			for (int z = 0; z < pixelsPerTile; ++z)
			{
				for (int x = 0; x < pixelsPerTile; ++x)
				{
					const int srcX = x + originOffset;
					const int srcZ = z + originOffset;

					// Destination pixel in the full image
					const int dstX = tx * pixelsPerTile + x;
					const int dstZ = tz * pixelsPerTile + z;

					// Image Y is inverted: top of image = north = max Z
					const int imgX = dstX;
					const int imgY = (imageSize - 1) - dstZ;

					if (imgX < 0 || imgX >= imageSize || imgY < 0 || imgY >= imageSize)
						continue;

					const PackedRgb color = buffer->colorMap.getData(srcX, srcZ);
					const float height = buffer->heightMap.getData(srcX, srcZ);
					const auto shader = buffer->shaderMap.getData(srcX, srcZ);
					//get shader to color pixel

					const int pixelIndex = imgY * imageSize + imgX;
					const int colorOffset = pixelIndex * 3;

					colorPixels[colorOffset + 0] = color.r;
					colorPixels[colorOffset + 1] = color.g;
					colorPixels[colorOffset + 2] = color.b;

					heightPixels[pixelIndex] = height;

					if (height < minHeight) minHeight = height;
					if (height > maxHeight) maxHeight = height;
				}
			}
		}
	}

	printf("\r  Generated %d tiles. Height range: %.1f to %.1f meters\n", totalTiles, minHeight, maxHeight);

	// Apply hillshading for 3D terrain effect
	printf("  Applying hillshading...\n");
	applyHillshading(heightPixels, colorPixels, imageSize, imageSize, mapWidth);

	// Apply water rendering
	if (terrainTemplate->getUseGlobalWaterTable())
	{
		const float waterHeight = terrainTemplate->getGlobalWaterTableHeight();
		printf("  Rendering water (global water table at %.1f meters)...\n", waterHeight);

		for (int y = 0; y < imageSize; ++y)
		{
			for (int x = 0; x < imageSize; ++x)
			{
				const int idx = y * imageSize + x;
				const float h = heightPixels[idx];

				if (h <= waterHeight)
				{
					const int offset = idx * 3;
					const float depth = waterHeight - h;
					const float waterFactor = std::min(depth / 30.0f, 0.85f);

					// Water color: deep blue
					const float waterR = 15.0f;
					const float waterG = 50.0f;
					const float waterB = 110.0f;

					colorPixels[offset + 0] = static_cast<uint8>(colorPixels[offset + 0] * (1.0f - waterFactor) + waterR * waterFactor);
					colorPixels[offset + 1] = static_cast<uint8>(colorPixels[offset + 1] * (1.0f - waterFactor) + waterG * waterFactor);
					colorPixels[offset + 2] = static_cast<uint8>(colorPixels[offset + 2] * (1.0f - waterFactor) + waterB * waterFactor);
				}
			}
		}
	}


	// Save output image
	char outputPath[512];
	_snprintf(outputPath, sizeof(outputPath), "%s/%s.tga", config.outputDir.c_str(), outputName);

	printf("  Saving: %s\n", outputPath);
	const bool saved = saveTGA(colorPixels, imageSize, imageSize, 3, outputPath);

	// Cleanup
	delete[] colorPixels;
	delete[] heightPixels;
	AppearanceTemplateList::release(at);

	if (saved)
		printf("  SUCCESS: %s (%dx%d)\n", outputPath, imageSize, imageSize);
	else
		printf("  ERROR: Failed to save %s\n", outputPath);

	return saved;
}


// ======================================================================
// GPU Rendered Mode
// ======================================================================


bool SwgMapRasterizer::renderTerrainGPU(const Config& config, const char* terrainFile, const char* outputName)
{
	if (!s_graphicsInitialized)
	{
		printf("  ERROR: Graphics not initialized. Cannot use GPU rendered mode.\n");
		printf("         Falling back to colormap mode.\n");
		return renderTerrainColormap(config, terrainFile, outputName);
	}

	printf("  Loading terrain for GPU rendering: %s\n", terrainFile);

	// Load the terrain template (client-side version for rendering)
	const AppearanceTemplate* at = AppearanceTemplateList::fetch(terrainFile);
	if (!at)
	{
		printf("  ERROR: Failed to load terrain template '%s'.\n", terrainFile);
		return false;
	}

	const ProceduralTerrainAppearanceTemplate* terrainTemplate = 
		dynamic_cast<const ProceduralTerrainAppearanceTemplate*>(at);

	if (!terrainTemplate)
	{
		printf("  ERROR: '%s' is not a procedural terrain file.\n", terrainFile);
		AppearanceTemplateList::release(at);
		return false;
	}

	const float mapWidth = terrainTemplate->getMapWidthInMeters();
	printf("  Map width: %.0f meters\n", mapWidth);

	// Create terrain appearance from the template
	Appearance* appearance = at->createAppearance();
	if (!appearance)
	{
		printf("  ERROR: Failed to create terrain appearance.\n");
		AppearanceTemplateList::release(at);
		return false;
	}

	// Create terrain object
	TerrainObject* terrainObject = new TerrainObject();
	terrainObject->setAppearance(appearance);

	// Create object list and camera
	ObjectList* objectList = new ObjectList(1);
	objectList->addObject(terrainObject);

	ObjectListCamera* camera = new ObjectListCamera(1);
	camera->addObjectList(objectList);
	camera->setViewport(0, 0, config.imageSize, config.imageSize);

	// Render each tile
	const int tilesPerSide = config.tilesPerSide;
	const int tilePixelSize = config.imageSize / tilesPerSide;
	int tilesDone = 0;
	const int totalTiles = tilesPerSide * tilesPerSide;
	bool allOk = true;

	for (int tz = 0; tz < tilesPerSide; ++tz)
	{
		for (int tx = 0; tx < tilesPerSide; ++tx)
		{
			++tilesDone;
			printf("\r  Rendering tile [%d/%d] (%d,%d)...", tilesDone, totalTiles, tx, tz);
			fflush(stdout);

			char tilePath[512];
			if (tilesPerSide == 1)
			{
				_snprintf(tilePath, sizeof(tilePath), "%s/%s.tga", config.outputDir.c_str(), outputName);
			}
			else
			{
				_snprintf(tilePath, sizeof(tilePath), "%s/%s_tile_%d_%d.tga", 
				         config.outputDir.c_str(), outputName, tx, tz);
			}

			if (!renderTile(camera, terrainObject, mapWidth, tx, tz, tilesPerSide,
			                config.cameraHeight, tilePixelSize, tilePath))
			{
				printf("\n  WARNING: Failed to render tile (%d,%d)\n", tx, tz);
				allOk = false;
			}
		}
	}

	printf("\r  Rendered %d/%d tiles successfully.        \n", tilesDone, totalTiles);

	// Cleanup
	camera->removeObjectList(objectList);
	delete camera;
	objectList->removeObject(terrainObject);
	delete objectList;
	delete terrainObject;
	AppearanceTemplateList::release(at);

	return allOk;
}

// ----------------------------------------------------------------------

bool SwgMapRasterizer::renderTile(
	ObjectListCamera* camera,
	TerrainObject* terrain,
	float mapWidth,
	int tileX,
	int tileZ,
	int tilesPerSide,
	float cameraHeight,
	int tilePixelSize,
	const char* outputPath
)
{
	const float tileWidth = mapWidth / tilesPerSide;
	const float halfTile  = tileWidth / 2.0f;

	// Center of this tile in world space
	const float centerX = -mapWidth / 2.0f + (tileX + 0.5f) * tileWidth;
	const float centerZ = -mapWidth / 2.0f + (tileZ + 0.5f) * tileWidth;

	// Set camera transform: position high above, looking straight down
	// Camera K (forward/Z) = world -Y (down)
	// Camera J (up/Y)      = world +Z (north) 
	// Camera I (right/X)   = world +X (east)
	Transform cameraTransform;
	cameraTransform.setLocalFrameKJ_p(
		Vector(0.0f, -1.0f, 0.0f),   // K = look down
		Vector(0.0f,  0.0f, 1.0f)    // J = north is up
	);
	cameraTransform.setPosition_p(Vector(centerX, cameraHeight, centerZ));
	camera->setTransform_o2p(cameraTransform);

	// Set orthographic projection covering the tile area
	// In camera space: X = world X, Y = world Z
	camera->setNearPlane(1.0f);
	camera->setFarPlane(cameraHeight + 5000.0f);
	camera->setParallelProjection(-halfTile, halfTile, halfTile, -halfTile);

	// Warm up terrain - call alter to trigger chunk generation
	for (int warmup = 0; warmup < 5; ++warmup)
	{
		terrain->alter(0.1f);
		Os::update();
	}

	// Render the scene
	camera->renderScene();
	return Graphics::screenShot(outputPath);
}

// ======================================================================
// TGA file writer
// ======================================================================

bool SwgMapRasterizer::saveTGA(const uint8* pixels, int width, int height, int channels, const char* filename)
{
	// Ensure output directory exists
	std::string path(filename);
	size_t lastSlash = path.find_last_of("/\\");
	if (lastSlash != std::string::npos)
	{
		std::string dir = path.substr(0, lastSlash);
		_mkdir(dir.c_str());
	}

	FILE* fp = fopen(filename, "wb");
	if (!fp)
	{
		printf("  ERROR: Cannot open file for writing: %s\n", filename);
		return false;
	}

	// TGA header
	uint8 header[18];
	memset(header, 0, sizeof(header));
	header[2]  = 2;                              // Uncompressed RGB
	header[12] = static_cast<uint8>(width & 0xFF);
	header[13] = static_cast<uint8>((width >> 8) & 0xFF);
	header[14] = static_cast<uint8>(height & 0xFF);
	header[15] = static_cast<uint8>((height >> 8) & 0xFF);
	header[16] = static_cast<uint8>(channels * 8); // Bits per pixel (24 or 32)
	header[17] = (channels == 4) ? 8 : 0;        // Image descriptor (alpha bits)

	fwrite(header, 1, sizeof(header), fp);

	// TGA stores pixels bottom-to-top, in BGR order
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			const int srcOffset = (y * width + x) * channels;
			uint8 bgr[4];

			if (channels >= 3)
			{
				bgr[0] = pixels[srcOffset + 2]; // B
				bgr[1] = pixels[srcOffset + 1]; // G
				bgr[2] = pixels[srcOffset + 0]; // R
				if (channels == 4)
					bgr[3] = pixels[srcOffset + 3]; // A
			}
			else
			{
				bgr[0] = bgr[1] = bgr[2] = pixels[srcOffset];
			}

			fwrite(bgr, 1, channels, fp);
		}
	}

	// TGA footer
	const char tgaFooter[] = "\0\0\0\0\0\0\0\0TRUEVISION-XFILE.\0";
	fwrite(tgaFooter, 1, 26, fp);

	fclose(fp);
	return true;
}

// ======================================================================
// Hillshading: applies directional lighting based on terrain normals
// to give the colormap a 3D appearance
// ======================================================================

void SwgMapRasterizer::applyHillshading(
	const float* heightMap,
	uint8* colorPixels,
	int width,
	int height,
	float mapWidth
)
{
	if (!heightMap || !colorPixels || width < 3 || height < 3)
		return;

	const float metersPerPixel = mapWidth / static_cast<float>(width);

	// Light direction (sun from upper-left, slightly elevated)
	// Normalized direction vector: (-1, -1, 2) normalized
	const float lightLen = sqrtf(1.0f + 1.0f + 4.0f);
	const float lightX = -1.0f / lightLen;
	const float lightZ = -1.0f / lightLen;
	const float lightY =  2.0f / lightLen;

	// Ambient + diffuse lighting parameters
	const float ambient  = 0.35f;
	const float diffuse  = 0.65f;

	for (int y = 1; y < height - 1; ++y)
	{
		for (int x = 1; x < width - 1; ++x)
		{
			// Calculate surface normal using central differences
			const float hL = heightMap[y * width + (x - 1)];
			const float hR = heightMap[y * width + (x + 1)];
			const float hD = heightMap[(y + 1) * width + x]; // down in image = south
			const float hU = heightMap[(y - 1) * width + x]; // up in image = north

			// Surface normal: cross product of tangent vectors
			// dX = (2*metersPerPixel, 0, hR - hL)
			// dZ = (0, 2*metersPerPixel, hU - hD)
			// Normal = dX cross dZ = (-2m*(hR-hL), -2m*(hU-hD), 4m^2)
			// But in our image coords, we need to be careful:
			// Image Y up = north, so hU (y-1) is north

			const float scale = 2.0f * metersPerPixel;
			float nx = -(hR - hL) / scale;
			float nz = -(hU - hD) / scale;
			float ny = 1.0f;

			// Normalize
			const float nLen = sqrtf(nx * nx + ny * ny + nz * nz);
			if (nLen > 0.0001f)
			{
				nx /= nLen;
				ny /= nLen;
				nz /= nLen;
			}

			// Dot product with light direction
			float dot = nx * lightX + ny * lightY + nz * lightZ;
			dot = std::max(0.0f, std::min(1.0f, dot));

			// Compute lighting factor
			float lighting = ambient + diffuse * dot;
			lighting = std::max(0.0f, std::min(1.5f, lighting)); // Allow slight over-brightening

			// Apply to color pixels
			const int offset = (y * width + x) * 3;
			for (int c = 0; c < 3; ++c)
			{
				float val = colorPixels[offset + c] * lighting;
				colorPixels[offset + c] = static_cast<uint8>(std::max(0.0f, std::min(255.0f, val)));
			}
		}
	}
}

// ======================================================================
