// ======================================================================
//
// SwgCuiTerraforming.cpp
// copyright 2026 Titan
//
// Comprehensive city terraforming UI implementation
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiTerraforming.h"

#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientTerrain/CityTerrainLayerManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "sharedFile/TreeFile.h"
#include "sharedMath/Vector.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/CityTerrainMessages.h"
#include "sharedObject/Object.h"
#include "sharedTerrain/TerrainObject.h"

#include "UIButton.h"
#include "UICheckbox.h"
#include "UIComboBox.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIImage.h"
#include "UIList.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UISliderbar.h"
#include "UIText.h"
#include "UITextbox.h"

#include <algorithm>
#include <cstdlib>
#include <ctime>

// ======================================================================

namespace
{
	const int32 MODIFICATION_TYPE_SHADER_CIRCLE = 0;
	const int32 MODIFICATION_TYPE_SHADER_LINE = 1;
	const int32 MODIFICATION_TYPE_HEIGHT_FLATTEN = 2;
	const int32 MODIFICATION_TYPE_HEIGHT_RAISE = 3;
	const int32 MODIFICATION_TYPE_HEIGHT_LOWER = 4;
	const int32 MODIFICATION_TYPE_HEIGHT_SMOOTH = 5;
	const int32 MODIFICATION_TYPE_HEIGHT_NOISE = 6;
	const int32 MODIFICATION_TYPE_AFFECTOR = 10;
}

// ======================================================================

// Static state preservation
int32 SwgCuiTerraforming::ms_savedCityId = 0;
SwgCuiTerraforming::TabId SwgCuiTerraforming::ms_savedTab = TAB_SHADER_PAINT;
int SwgCuiTerraforming::ms_savedShaderIndex = 0;
long SwgCuiTerraforming::ms_savedPaintRadius = 15;
long SwgCuiTerraforming::ms_savedPaintBlend = 8;
long SwgCuiTerraforming::ms_savedPaintFeather = 50;
int SwgCuiTerraforming::ms_savedHeightMode = 0;
long SwgCuiTerraforming::ms_savedHeightValue = 0;
long SwgCuiTerraforming::ms_savedHeightRadius = 30;
long SwgCuiTerraforming::ms_savedHeightStrength = 100;
long SwgCuiTerraforming::ms_savedHeightFeather = 75;
long SwgCuiTerraforming::ms_savedRoadWidth = 6;
int SwgCuiTerraforming::ms_savedRoadShaderIndex = 0;
int SwgCuiTerraforming::ms_savedAffectorType = 0;
long SwgCuiTerraforming::ms_savedAffectorRadius = 25;
long SwgCuiTerraforming::ms_savedAffectorIntensity = 100;
long SwgCuiTerraforming::ms_savedAffectorFeather = 50;
bool SwgCuiTerraforming::ms_hasRestoredState = false;
SwgCuiTerraforming * SwgCuiTerraforming::ms_activeInstance = 0;

// ======================================================================

SwgCuiTerraforming::SwgCuiTerraforming(UIPage & page) :
	CuiMediator("SwgCuiTerraforming", page),
	UIEventCallback(),
	m_pageShaderPaint(nullptr),
	m_pageHeightEdit(nullptr),
	m_pageRoads(nullptr),
	m_pageAffectors(nullptr),
	m_pageRegions(nullptr),
	m_tabShaderPaint(nullptr),
	m_tabHeightEdit(nullptr),
	m_tabRoads(nullptr),
	m_tabAffectors(nullptr),
	m_tabRegions(nullptr),
	m_textCityName(nullptr),
	m_buttonClose(nullptr),
	m_comboShader(nullptr),
	m_sliderPaintRadius(nullptr),
	m_textPaintRadius(nullptr),
	m_sliderPaintBlend(nullptr),
	m_textPaintBlend(nullptr),
	m_sliderPaintFeather(nullptr),
	m_textPaintFeather(nullptr),
	m_checkPaintPreview(nullptr),
	m_imageShaderPreview(nullptr),
	m_buttonPaintApply(nullptr),
	m_comboHeightMode(nullptr),
	m_sliderHeightValue(nullptr),
	m_textHeightValue(nullptr),
	m_sliderHeightRadius(nullptr),
	m_textHeightRadius(nullptr),
	m_sliderHeightStrength(nullptr),
	m_textHeightStrength(nullptr),
	m_sliderHeightFeather(nullptr),
	m_textHeightFeather(nullptr),
	m_buttonHeightSample(nullptr),
	m_buttonHeightApply(nullptr),
	m_sliderRoadWidth(nullptr),
	m_textRoadWidth(nullptr),
	m_comboRoadShader(nullptr),
	m_checkRoadSmooth(nullptr),
	m_checkRoadFlatten(nullptr),
	m_buttonRoadStart(nullptr),
	m_buttonRoadEnd(nullptr),
	m_buttonRoadApply(nullptr),
	m_textRoadStatus(nullptr),
	m_textStartPos(nullptr),
	m_textEndPos(nullptr),
	m_comboAffectorType(nullptr),
	m_sliderAffectorRadius(nullptr),
	m_textAffectorRadius(nullptr),
	m_sliderAffectorIntensity(nullptr),
	m_textAffectorIntensity(nullptr),
	m_sliderAffectorFeather(nullptr),
	m_textAffectorFeather(nullptr),
	m_comboAffectorParam(nullptr),
	m_sliderAffectorParam(nullptr),
	m_textAffectorParam(nullptr),
	m_buttonAffectorApply(nullptr),
	m_listRegions(nullptr),
	m_buttonRegionDelete(nullptr),
	m_buttonRegionDeleteAll(nullptr),
	m_textRegionInfo(nullptr),
	m_textStatus(nullptr),
	m_buttonUndo(nullptr),
	m_buttonResetTerrain(nullptr),
	m_cityId(0),
	m_cityRadius(0),
	m_cityName(),
	m_currentTab(TAB_SHADER_PAINT),
	m_shaderTemplates(),
	m_shaderNames(),
	m_roadStartX(0.0f),
	m_roadStartZ(0.0f),
	m_roadEndX(0.0f),
	m_roadEndZ(0.0f),
	m_hasRoadStart(false),
	m_hasRoadEnd(false),
	m_regions(),
	m_appliedRegionIds(),
	m_callback(new MessageDispatch::Callback)
{
	// Get all widgets from CodeData
	getCodeDataObject(TUIPage, m_pageShaderPaint, "pageShaderPaint", true);
	getCodeDataObject(TUIPage, m_pageHeightEdit, "pageHeightEdit", true);
	getCodeDataObject(TUIPage, m_pageRoads, "pageRoads", true);
	getCodeDataObject(TUIPage, m_pageAffectors, "pageAffectors", true);
	getCodeDataObject(TUIPage, m_pageRegions, "pageRegions", true);

	getCodeDataObject(TUIButton, m_tabShaderPaint, "tabShaderPaint", true);
	getCodeDataObject(TUIButton, m_tabHeightEdit, "tabHeightEdit", true);
	getCodeDataObject(TUIButton, m_tabRoads, "tabRoads", true);
	getCodeDataObject(TUIButton, m_tabAffectors, "tabAffectors", true);
	getCodeDataObject(TUIButton, m_tabRegions, "tabRegions", true);

	getCodeDataObject(TUIText, m_textCityName, "textCityName", false);
	getCodeDataObject(TUIButton, m_buttonClose, "buttonClose", true);

	// Shader Paint tab
	getCodeDataObject(TUIComboBox, m_comboShader, "comboShader", true);
	getCodeDataObject(TUISliderbar, m_sliderPaintRadius, "sliderPaintRadius", true);
	getCodeDataObject(TUITextbox, m_textPaintRadius, "textPaintRadius", true);
	getCodeDataObject(TUISliderbar, m_sliderPaintBlend, "sliderPaintBlend", true);
	getCodeDataObject(TUITextbox, m_textPaintBlend, "textPaintBlend", true);
	getCodeDataObject(TUISliderbar, m_sliderPaintFeather, "sliderPaintFeather", true);
	getCodeDataObject(TUITextbox, m_textPaintFeather, "textPaintFeather", true);
	getCodeDataObject(TUICheckbox, m_checkPaintPreview, "checkPaintPreview", false);
	getCodeDataObject(TUIImage, m_imageShaderPreview, "imageShaderPreview", false);
	getCodeDataObject(TUIButton, m_buttonPaintApply, "buttonPaintApply", true);

	// Height Edit tab
	getCodeDataObject(TUIComboBox, m_comboHeightMode, "comboHeightMode", true);
	getCodeDataObject(TUISliderbar, m_sliderHeightValue, "sliderHeightValue", true);
	getCodeDataObject(TUITextbox, m_textHeightValue, "textHeightValue", true);
	getCodeDataObject(TUISliderbar, m_sliderHeightRadius, "sliderHeightRadius", true);
	getCodeDataObject(TUITextbox, m_textHeightRadius, "textHeightRadius", true);
	getCodeDataObject(TUISliderbar, m_sliderHeightStrength, "sliderHeightStrength", true);
	getCodeDataObject(TUITextbox, m_textHeightStrength, "textHeightStrength", true);
	getCodeDataObject(TUISliderbar, m_sliderHeightFeather, "sliderHeightFeather", true);
	getCodeDataObject(TUITextbox, m_textHeightFeather, "textHeightFeather", true);
	getCodeDataObject(TUIButton, m_buttonHeightSample, "buttonHeightSample", false);
	getCodeDataObject(TUIButton, m_buttonHeightApply, "buttonHeightApply", true);

	// Roads tab
	getCodeDataObject(TUISliderbar, m_sliderRoadWidth, "sliderRoadWidth", true);
	getCodeDataObject(TUITextbox, m_textRoadWidth, "textRoadWidth", true);
	getCodeDataObject(TUIComboBox, m_comboRoadShader, "comboRoadShader", true);
	getCodeDataObject(TUICheckbox, m_checkRoadSmooth, "checkRoadSmooth", false);
	getCodeDataObject(TUICheckbox, m_checkRoadFlatten, "checkRoadFlatten", false);
	getCodeDataObject(TUIButton, m_buttonRoadStart, "buttonRoadStart", true);
	getCodeDataObject(TUIButton, m_buttonRoadEnd, "buttonRoadEnd", true);
	getCodeDataObject(TUIButton, m_buttonRoadApply, "buttonRoadApply", true);
	getCodeDataObject(TUIText, m_textRoadStatus, "textRoadStatus", false);
	getCodeDataObject(TUIText, m_textStartPos, "textStartPos", false);
	getCodeDataObject(TUIText, m_textEndPos, "textEndPos", false);

	// Affectors tab
	getCodeDataObject(TUIComboBox, m_comboAffectorType, "comboAffectorType", true);
	getCodeDataObject(TUISliderbar, m_sliderAffectorRadius, "sliderAffectorRadius", true);
	getCodeDataObject(TUITextbox, m_textAffectorRadius, "textAffectorRadius", true);
	getCodeDataObject(TUISliderbar, m_sliderAffectorIntensity, "sliderAffectorIntensity", true);
	getCodeDataObject(TUITextbox, m_textAffectorIntensity, "textAffectorIntensity", true);
	getCodeDataObject(TUISliderbar, m_sliderAffectorFeather, "sliderAffectorFeather", true);
	getCodeDataObject(TUITextbox, m_textAffectorFeather, "textAffectorFeather", true);
	getCodeDataObject(TUIComboBox, m_comboAffectorParam, "comboAffectorParam", false);
	getCodeDataObject(TUISliderbar, m_sliderAffectorParam, "sliderAffectorParam", false);
	getCodeDataObject(TUITextbox, m_textAffectorParam, "textAffectorParam", false);
	getCodeDataObject(TUIButton, m_buttonAffectorApply, "buttonAffectorApply", true);

	// Regions tab
	getCodeDataObject(TUIList, m_listRegions, "listRegions", true);
	getCodeDataObject(TUIButton, m_buttonRegionDelete, "buttonRegionDelete", true);
	getCodeDataObject(TUIButton, m_buttonRegionDeleteAll, "buttonRegionDeleteAll", true);
	getCodeDataObject(TUIText, m_textRegionInfo, "textRegionInfo", false);

	// Common
	getCodeDataObject(TUIText, m_textStatus, "textStatus", false);
	getCodeDataObject(TUIButton, m_buttonUndo, "buttonUndo", false);
	getCodeDataObject(TUIButton, m_buttonResetTerrain, "buttonResetTerrain", false);

	// Register callbacks
	if (m_buttonClose)
		registerMediatorObject(*m_buttonClose, true);
	if (m_buttonUndo)
		registerMediatorObject(*m_buttonUndo, true);
	if (m_buttonResetTerrain)
		registerMediatorObject(*m_buttonResetTerrain, true);

	// Tab buttons
	if (m_tabShaderPaint)
		registerMediatorObject(*m_tabShaderPaint, true);
	if (m_tabHeightEdit)
		registerMediatorObject(*m_tabHeightEdit, true);
	if (m_tabRoads)
		registerMediatorObject(*m_tabRoads, true);
	if (m_tabAffectors)
		registerMediatorObject(*m_tabAffectors, true);
	if (m_tabRegions)
		registerMediatorObject(*m_tabRegions, true);

	// Shader Paint controls
	if (m_comboShader)
		registerMediatorObject(*m_comboShader, true);
	if (m_sliderPaintRadius)
		registerMediatorObject(*m_sliderPaintRadius, true);
	if (m_sliderPaintBlend)
		registerMediatorObject(*m_sliderPaintBlend, true);
	if (m_sliderPaintFeather)
		registerMediatorObject(*m_sliderPaintFeather, true);
	if (m_checkPaintPreview)
		registerMediatorObject(*m_checkPaintPreview, true);
	if (m_buttonPaintApply)
		registerMediatorObject(*m_buttonPaintApply, true);

	// Height Edit controls
	if (m_comboHeightMode)
		registerMediatorObject(*m_comboHeightMode, true);
	if (m_sliderHeightValue)
		registerMediatorObject(*m_sliderHeightValue, true);
	if (m_sliderHeightRadius)
		registerMediatorObject(*m_sliderHeightRadius, true);
	if (m_sliderHeightStrength)
		registerMediatorObject(*m_sliderHeightStrength, true);
	if (m_sliderHeightFeather)
		registerMediatorObject(*m_sliderHeightFeather, true);
	if (m_buttonHeightSample)
		registerMediatorObject(*m_buttonHeightSample, true);
	if (m_buttonHeightApply)
		registerMediatorObject(*m_buttonHeightApply, true);

	// Roads controls
	if (m_sliderRoadWidth)
		registerMediatorObject(*m_sliderRoadWidth, true);
	if (m_comboRoadShader)
		registerMediatorObject(*m_comboRoadShader, true);
	if (m_checkRoadSmooth)
		registerMediatorObject(*m_checkRoadSmooth, true);
	if (m_checkRoadFlatten)
		registerMediatorObject(*m_checkRoadFlatten, true);
	if (m_buttonRoadStart)
		registerMediatorObject(*m_buttonRoadStart, true);
	if (m_buttonRoadEnd)
		registerMediatorObject(*m_buttonRoadEnd, true);
	if (m_buttonRoadApply)
		registerMediatorObject(*m_buttonRoadApply, true);

	// Affectors controls
	if (m_comboAffectorType)
		registerMediatorObject(*m_comboAffectorType, true);
	if (m_sliderAffectorRadius)
		registerMediatorObject(*m_sliderAffectorRadius, true);
	if (m_sliderAffectorIntensity)
		registerMediatorObject(*m_sliderAffectorIntensity, true);
	if (m_sliderAffectorFeather)
		registerMediatorObject(*m_sliderAffectorFeather, true);
	if (m_sliderAffectorParam)
		registerMediatorObject(*m_sliderAffectorParam, true);
	if (m_buttonAffectorApply)
		registerMediatorObject(*m_buttonAffectorApply, true);

	// Regions controls
	if (m_listRegions)
		registerMediatorObject(*m_listRegions, true);
	if (m_buttonRegionDelete)
		registerMediatorObject(*m_buttonRegionDelete, true);
	if (m_buttonRegionDeleteAll)
		registerMediatorObject(*m_buttonRegionDeleteAll, true);

	REPORT_LOG(true, ("[Titan] SwgCuiTerraforming: Constructor complete\n"));
}

// ----------------------------------------------------------------------

SwgCuiTerraforming::~SwgCuiTerraforming()
{
	delete m_callback;
	m_callback = nullptr;

	m_pageShaderPaint = nullptr;
	m_pageHeightEdit = nullptr;
	m_pageRoads = nullptr;
	m_pageAffectors = nullptr;
	m_pageRegions = nullptr;
	m_tabShaderPaint = nullptr;
	m_tabHeightEdit = nullptr;
	m_tabRoads = nullptr;
	m_tabAffectors = nullptr;
	m_tabRegions = nullptr;
}

// ----------------------------------------------------------------------

SwgCuiTerraforming * SwgCuiTerraforming::createInto(UIPage & parent)
{
	UIPage * const page = NON_NULL(dynamic_cast<UIPage *>(parent.GetChild("terraformingWindow")));
	return new SwgCuiTerraforming(*page);
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::performActivate()
{
	REPORT_LOG(true, ("[Titan] SwgCuiTerraforming::performActivate\n"));

	// Register for region change callbacks
	ms_activeInstance = this;
	if (CityTerrainLayerManager::isInstalled())
	{
		CityTerrainLayerManager::setRegionChangeCallback(&SwgCuiTerraforming::onRegionChangeCallback);
	}

	// Get pending city ID
	if (CityTerrainLayerManager::isInstalled())
	{
		int32 pendingCityId = CityTerrainLayerManager::getPendingCityId();
		if (pendingCityId != 0)
		{
			m_cityId = pendingCityId;
			m_cityRadius = CityTerrainLayerManager::getCityRadius(m_cityId);
		}
	}

	// Request terrain regions sync from server
	if (m_cityId != 0)
	{
		REPORT_LOG(true, ("[Titan] SwgCuiTerraforming::performActivate - requesting sync for city %d\n", m_cityId));
		CityTerrainSyncRequestMessage const syncMsg(m_cityId);
		GameNetwork::send(syncMsg, true);
	}

	loadShaderList();
	restoreUIState();
	updateTabVisuals();
	updateRoadStatus();
	refreshRegionList();
	clearStatus();
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::performDeactivate()
{
	REPORT_LOG(true, ("[Titan] SwgCuiTerraforming::performDeactivate\n"));
	saveUIState();

	// Unregister callback
	ms_activeInstance = 0;
	if (CityTerrainLayerManager::isInstalled())
	{
		CityTerrainLayerManager::clearRegionChangeCallback();
	}
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::onRegionChangeCallback(int32 cityId)
{
	REPORT_LOG(true, ("[Titan] SwgCuiTerraforming::onRegionChangeCallback: cityId=%d activeInstance=%p\n",
		cityId, ms_activeInstance));

	if (ms_activeInstance && ms_activeInstance->m_cityId == cityId)
	{
		ms_activeInstance->refreshRegionList();
	}
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::setCityId(int32 cityId)
{
	m_cityId = cityId;
	if (CityTerrainLayerManager::isInstalled())
	{
		m_cityRadius = CityTerrainLayerManager::getCityRadius(cityId);
	}
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::setCityName(const std::string & name)
{
	m_cityName = name;
	if (m_textCityName)
	{
		m_textCityName->SetLocalText(Unicode::narrowToWide(name));
	}
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::setCityRadius(int32 radius)
{
	m_cityRadius = radius;
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::OnButtonPressed(UIWidget * context)
{
	if (!context)
		return;

	// Close button
	if (context == m_buttonClose)
	{
		deactivate();
		return;
	}

	// Undo
	if (context == m_buttonUndo)
	{
		undoLastModification();
		return;
	}

	// Tab buttons
	if (context == m_tabShaderPaint)
	{
		switchToTab(TAB_SHADER_PAINT);
		return;
	}
	if (context == m_tabHeightEdit)
	{
		switchToTab(TAB_HEIGHT_EDIT);
		return;
	}
	if (context == m_tabRoads)
	{
		switchToTab(TAB_ROADS);
		return;
	}
	if (context == m_tabAffectors)
	{
		switchToTab(TAB_AFFECTORS);
		return;
	}
	if (context == m_tabRegions)
	{
		switchToTab(TAB_REGIONS);
		refreshRegionList();
		return;
	}

	// Apply buttons
	if (context == m_buttonPaintApply)
	{
		applyShaderPaint();
		return;
	}
	if (context == m_buttonHeightApply)
	{
		applyHeightEdit();
		return;
	}
	if (context == m_buttonRoadApply)
	{
		applyRoad();
		return;
	}
	if (context == m_buttonAffectorApply)
	{
		applyAffector();
		return;
	}

	// Height sample
	if (context == m_buttonHeightSample)
	{
		sampleCurrentHeight();
		return;
	}

	// Road markers
	if (context == m_buttonRoadStart)
	{
		setRoadStart();
		return;
	}
	if (context == m_buttonRoadEnd)
	{
		setRoadEnd();
		return;
	}

	// Region management
	if (context == m_buttonRegionDelete)
	{
		deleteSelectedRegion();
		return;
	}
	if (context == m_buttonRegionDeleteAll)
	{
		deleteAllRegions();
		return;
	}

	// Reset Terrain
	if (context == m_buttonResetTerrain)
	{
		showResetTerrainConfirmation();
		return;
	}
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::OnCheckboxSet(UIWidget * context)
{
	if (context == m_checkPaintPreview)
	{
		// Enable preview rendering
	}
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::OnCheckboxUnset(UIWidget * context)
{
	if (context == m_checkPaintPreview)
	{
		// Disable preview rendering
	}
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::OnSliderbarChanged(UIWidget * context)
{
	// Shader Paint sliders
	if (context == m_sliderPaintRadius)
	{
		syncSliderToText(m_sliderPaintRadius, m_textPaintRadius);
		return;
	}
	if (context == m_sliderPaintBlend)
	{
		syncSliderToText(m_sliderPaintBlend, m_textPaintBlend);
		return;
	}
	if (context == m_sliderPaintFeather)
	{
		syncSliderToText(m_sliderPaintFeather, m_textPaintFeather);
		return;
	}

	// Height Edit sliders
	if (context == m_sliderHeightValue)
	{
		syncSliderToText(m_sliderHeightValue, m_textHeightValue);
		return;
	}
	if (context == m_sliderHeightRadius)
	{
		syncSliderToText(m_sliderHeightRadius, m_textHeightRadius);
		return;
	}
	if (context == m_sliderHeightStrength)
	{
		syncSliderToText(m_sliderHeightStrength, m_textHeightStrength);
		return;
	}
	if (context == m_sliderHeightFeather)
	{
		syncSliderToText(m_sliderHeightFeather, m_textHeightFeather);
		return;
	}

	// Roads slider
	if (context == m_sliderRoadWidth)
	{
		syncSliderToText(m_sliderRoadWidth, m_textRoadWidth);
		return;
	}

	// Affector sliders
	if (context == m_sliderAffectorRadius)
	{
		syncSliderToText(m_sliderAffectorRadius, m_textAffectorRadius);
		return;
	}
	if (context == m_sliderAffectorIntensity)
	{
		syncSliderToText(m_sliderAffectorIntensity, m_textAffectorIntensity);
		return;
	}
	if (context == m_sliderAffectorFeather)
	{
		syncSliderToText(m_sliderAffectorFeather, m_textAffectorFeather);
		return;
	}
	if (context == m_sliderAffectorParam)
	{
		syncSliderToText(m_sliderAffectorParam, m_textAffectorParam);
		return;
	}
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::OnGenericSelectionChanged(UIWidget * context)
{
	if (context == m_comboShader)
	{
		updateShaderPreview();
		return;
	}
	if (context == m_listRegions)
	{
		updateRegionInfo();
		if (m_buttonRegionDelete)
		{
			m_buttonRegionDelete->SetEnabled(m_listRegions && m_listRegions->GetLastSelectedRow() >= 0);
		}
		return;
	}
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::switchToTab(TabId tab)
{
	m_currentTab = tab;
	updateTabVisuals();
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::updateTabVisuals()
{
	if (m_pageShaderPaint)
		m_pageShaderPaint->SetVisible(m_currentTab == TAB_SHADER_PAINT);
	if (m_pageHeightEdit)
		m_pageHeightEdit->SetVisible(m_currentTab == TAB_HEIGHT_EDIT);
	if (m_pageRoads)
		m_pageRoads->SetVisible(m_currentTab == TAB_ROADS);
	if (m_pageAffectors)
		m_pageAffectors->SetVisible(m_currentTab == TAB_AFFECTORS);
	if (m_pageRegions)
		m_pageRegions->SetVisible(m_currentTab == TAB_REGIONS);

	// Update tab button states with visual feedback
	Unicode::String const selectedColorStr = Unicode::narrowToWide("#00FFFF");  // Cyan for selected
	Unicode::String const normalColorStr = Unicode::narrowToWide("#CCCCCC");    // Light gray for normal
	UILowerString const textColorProp("TextColor");

	if (m_tabShaderPaint)
	{
		m_tabShaderPaint->SetSelected(m_currentTab == TAB_SHADER_PAINT);
		m_tabShaderPaint->SetProperty(textColorProp,
			m_currentTab == TAB_SHADER_PAINT ? selectedColorStr : normalColorStr);
	}
	if (m_tabHeightEdit)
	{
		m_tabHeightEdit->SetSelected(m_currentTab == TAB_HEIGHT_EDIT);
		m_tabHeightEdit->SetProperty(textColorProp,
			m_currentTab == TAB_HEIGHT_EDIT ? selectedColorStr : normalColorStr);
	}
	if (m_tabRoads)
	{
		m_tabRoads->SetSelected(m_currentTab == TAB_ROADS);
		m_tabRoads->SetProperty(textColorProp,
			m_currentTab == TAB_ROADS ? selectedColorStr : normalColorStr);
	}
	if (m_tabAffectors)
	{
		m_tabAffectors->SetSelected(m_currentTab == TAB_AFFECTORS);
		m_tabAffectors->SetProperty(textColorProp,
			m_currentTab == TAB_AFFECTORS ? selectedColorStr : normalColorStr);
	}
	if (m_tabRegions)
	{
		m_tabRegions->SetSelected(m_currentTab == TAB_REGIONS);
		m_tabRegions->SetProperty(textColorProp,
			m_currentTab == TAB_REGIONS ? selectedColorStr : normalColorStr);
	}
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::loadShaderList()
{
	m_shaderTemplates.clear();
	m_shaderNames.clear();

	if (CityTerrainLayerManager::isInstalled())
	{
		CityTerrainLayerManager::getInstance().enumerateAvailableShaders(m_shaderTemplates, m_shaderNames);
	}

	// Populate shader combobox
	if (m_comboShader)
	{
		UIDataSource * ds = m_comboShader->GetDataSource();
		if (ds)
		{
			ds->Clear();
			for (size_t i = 0; i < m_shaderNames.size(); ++i)
			{
				UIData * data = new UIData();
				char idBuf[32];
				snprintf(idBuf, sizeof(idBuf), "%d", static_cast<int>(i));
				data->SetName(idBuf);
				data->SetProperty(UILowerString("Text"), Unicode::narrowToWide(m_shaderNames[i]));
				ds->AddChild(data);
			}
			m_comboShader->SetSelectedIndex(0);
		}
	}

	// Populate road shader combobox (same shaders)
	if (m_comboRoadShader)
	{
		UIDataSource * ds = m_comboRoadShader->GetDataSource();
		if (ds)
		{
			ds->Clear();
			for (size_t i = 0; i < m_shaderNames.size(); ++i)
			{
				UIData * data = new UIData();
				char idBuf[32];
				snprintf(idBuf, sizeof(idBuf), "%d", static_cast<int>(i));
				data->SetName(idBuf);
				data->SetProperty(UILowerString("Text"), Unicode::narrowToWide(m_shaderNames[i]));
				ds->AddChild(data);
			}
			m_comboRoadShader->SetSelectedIndex(0);
		}
	}

	REPORT_LOG(true, ("[Titan] SwgCuiTerraforming::loadShaderList - loaded %d shaders\n", static_cast<int>(m_shaderNames.size())));
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::updateShaderPreview()
{
	// Preview functionality - could show shader texture preview
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::syncSliderToText(UISliderbar * slider, UITextbox * textbox)
{
	if (!slider || !textbox)
		return;

	long value = slider->GetValue();
	char buf[32];
	snprintf(buf, sizeof(buf), "%ld", value);
	textbox->SetLocalText(Unicode::narrowToWide(buf));
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::syncTextToSlider(UITextbox * textbox, UISliderbar * slider)
{
	if (!slider || !textbox)
		return;

	Unicode::String text;
	textbox->GetLocalText(text);
	std::string narrow = Unicode::wideToNarrow(text);

	long value = atol(narrow.c_str());
	long lower = slider->GetLowerLimit();
	long upper = slider->GetUpperLimit();

	if (value < lower) value = lower;
	if (value > upper) value = upper;

	slider->SetValue(value, false);
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::applyShaderPaint()
{
	if (!validateCityPermission())
	{
		setStatus("You do not have permission to terraform this city.", true);
		return;
	}

	float centerX, centerZ;
	if (!validatePosition(centerX, centerZ))
	{
		setStatus("Invalid position. Move within city limits.", true);
		return;
	}

	int shaderIdx = m_comboShader ? m_comboShader->GetSelectedIndex() : 0;
	if (shaderIdx < 0 || shaderIdx >= static_cast<int>(m_shaderTemplates.size()))
	{
		setStatus("Please select a valid shader.", true);
		return;
	}

	long radius = m_sliderPaintRadius ? m_sliderPaintRadius->GetValue() : 15;
	long blend = m_sliderPaintBlend ? m_sliderPaintBlend->GetValue() : 8;
	long feather = m_sliderPaintFeather ? m_sliderPaintFeather->GetValue() : 50;

	std::string shaderTemplate = m_shaderTemplates[shaderIdx];
	float blendDist = static_cast<float>(blend);
	float featherMult = static_cast<float>(feather) / 100.0f;

	sendTerrainModification(MODIFICATION_TYPE_SHADER_CIRCLE, shaderTemplate,
		centerX, centerZ, static_cast<float>(radius),
		0.0f, 0.0f, featherMult, 0.0f, blendDist);

	// Create local region for immediate feedback
	std::string regionId = generateRegionId();
	m_appliedRegionIds.push_back(regionId);

	if (CityTerrainLayerManager::isInstalled())
	{
		CityTerrainLayerManager::TerrainRegion region;
		region.regionId = regionId;
		region.cityId = m_cityId;
		region.type = CityTerrainLayerManager::RT_SHADER_CIRCLE;
		region.shaderTemplate = shaderTemplate;
		region.centerX = centerX;
		region.centerZ = centerZ;
		region.radius = static_cast<float>(radius);
		region.endX = 0.0f;
		region.endZ = 0.0f;
		region.width = 0.0f;
		region.height = 0.0f;
		region.blendDistance = blendDist;
		region.cachedShader = nullptr;
		region.active = true;
		CityTerrainLayerManager::getInstance().addRegion(region);
	}

	setStatus("Shader paint applied successfully.");
	REPORT_LOG(true, ("[Titan] SwgCuiTerraforming::applyShaderPaint - shader=%s, pos=(%.1f, %.1f), radius=%ld\n",
		shaderTemplate.c_str(), centerX, centerZ, radius));
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::applyHeightEdit()
{
	if (!validateCityPermission())
	{
		setStatus("You do not have permission to terraform this city.", true);
		return;
	}

	float centerX, centerZ;
	if (!validatePosition(centerX, centerZ))
	{
		setStatus("Invalid position. Move within city limits.", true);
		return;
	}

	int mode = m_comboHeightMode ? m_comboHeightMode->GetSelectedIndex() : 0;
	long heightVal = m_sliderHeightValue ? m_sliderHeightValue->GetValue() : 0;
	long radius = m_sliderHeightRadius ? m_sliderHeightRadius->GetValue() : 30;
	long strength = m_sliderHeightStrength ? m_sliderHeightStrength->GetValue() : 100;
	long feather = m_sliderHeightFeather ? m_sliderHeightFeather->GetValue() : 75;

	int32 modType = MODIFICATION_TYPE_HEIGHT_FLATTEN;
	switch (mode)
	{
	case HM_FLATTEN: modType = MODIFICATION_TYPE_HEIGHT_FLATTEN; break;
	case HM_RAISE: modType = MODIFICATION_TYPE_HEIGHT_RAISE; break;
	case HM_LOWER: modType = MODIFICATION_TYPE_HEIGHT_LOWER; break;
	case HM_SMOOTH: modType = MODIFICATION_TYPE_HEIGHT_SMOOTH; break;
	case HM_NOISE: modType = MODIFICATION_TYPE_HEIGHT_NOISE; break;
	}

	float strengthMult = static_cast<float>(strength) / 100.0f;
	float featherMult = static_cast<float>(feather) / 100.0f;

	sendTerrainModification(modType, "",
		centerX, centerZ, static_cast<float>(radius),
		0.0f, 0.0f, strengthMult, static_cast<float>(heightVal), featherMult * 20.0f);

	// Create local region
	std::string regionId = generateRegionId();
	m_appliedRegionIds.push_back(regionId);

	if (CityTerrainLayerManager::isInstalled())
	{
		CityTerrainLayerManager::TerrainRegion region;
		region.regionId = regionId;
		region.cityId = m_cityId;
		region.type = CityTerrainLayerManager::RT_FLATTEN;
		region.shaderTemplate = "";
		region.centerX = centerX;
		region.centerZ = centerZ;
		region.radius = static_cast<float>(radius);
		region.endX = 0.0f;
		region.endZ = 0.0f;
		region.width = strengthMult;
		region.height = static_cast<float>(heightVal);
		region.blendDistance = featherMult * 20.0f;
		region.cachedShader = nullptr;
		region.active = true;
		CityTerrainLayerManager::getInstance().addRegion(region);
	}

	setStatus("Height modification applied successfully.");
	REPORT_LOG(true, ("[Titan] SwgCuiTerraforming::applyHeightEdit - mode=%d, height=%ld, pos=(%.1f, %.1f), radius=%ld\n",
		mode, heightVal, centerX, centerZ, radius));
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::applyRoad()
{
	if (!validateCityPermission())
	{
		setStatus("You do not have permission to terraform this city.", true);
		return;
	}

	if (!m_hasRoadStart || !m_hasRoadEnd)
	{
		setStatus("Please set both start and end points for the road.", true);
		return;
	}

	int shaderIdx = m_comboRoadShader ? m_comboRoadShader->GetSelectedIndex() : 0;
	if (shaderIdx < 0 || shaderIdx >= static_cast<int>(m_shaderTemplates.size()))
	{
		setStatus("Please select a valid road surface.", true);
		return;
	}

	long width = m_sliderRoadWidth ? m_sliderRoadWidth->GetValue() : 6;
	std::string shaderTemplate = m_shaderTemplates[shaderIdx];

	sendTerrainModification(MODIFICATION_TYPE_SHADER_LINE, shaderTemplate,
		m_roadStartX, m_roadStartZ, static_cast<float>(width),
		m_roadEndX, m_roadEndZ, static_cast<float>(width), 0.0f, 2.0f);

	// Create local region
	std::string regionId = generateRegionId();
	m_appliedRegionIds.push_back(regionId);

	if (CityTerrainLayerManager::isInstalled())
	{
		CityTerrainLayerManager::TerrainRegion region;
		region.regionId = regionId;
		region.cityId = m_cityId;
		region.type = CityTerrainLayerManager::RT_SHADER_LINE;
		region.shaderTemplate = shaderTemplate;
		region.centerX = m_roadStartX;
		region.centerZ = m_roadStartZ;
		region.radius = 0.0f;
		region.endX = m_roadEndX;
		region.endZ = m_roadEndZ;
		region.width = static_cast<float>(width);
		region.height = 0.0f;
		region.blendDistance = 2.0f;
		region.cachedShader = nullptr;
		region.active = true;
		CityTerrainLayerManager::getInstance().addRegion(region);
	}

	setStatus("Road created successfully.");
	clearRoadMarkers();

	REPORT_LOG(true, ("[Titan] SwgCuiTerraforming::applyRoad - shader=%s, start=(%.1f, %.1f), end=(%.1f, %.1f), width=%ld\n",
		shaderTemplate.c_str(), m_roadStartX, m_roadStartZ, m_roadEndX, m_roadEndZ, width));
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::applyAffector()
{
	if (!validateCityPermission())
	{
		setStatus("You do not have permission to terraform this city.", true);
		return;
	}

	float centerX, centerZ;
	if (!validatePosition(centerX, centerZ))
	{
		setStatus("Invalid position. Move within city limits.", true);
		return;
	}

	int affectorType = m_comboAffectorType ? m_comboAffectorType->GetSelectedIndex() : 0;
	long radius = m_sliderAffectorRadius ? m_sliderAffectorRadius->GetValue() : 25;
	long intensity = m_sliderAffectorIntensity ? m_sliderAffectorIntensity->GetValue() : 100;
	long feather = m_sliderAffectorFeather ? m_sliderAffectorFeather->GetValue() : 50;
	long param = m_sliderAffectorParam ? m_sliderAffectorParam->GetValue() : 50;

	float intensityMult = static_cast<float>(intensity) / 100.0f;
	float featherMult = static_cast<float>(feather) / 100.0f;
	float paramVal = static_cast<float>(param);

	sendTerrainModification(MODIFICATION_TYPE_AFFECTOR + affectorType, "",
		centerX, centerZ, static_cast<float>(radius),
		0.0f, 0.0f, intensityMult, paramVal, featherMult * 20.0f);

	std::string regionId = generateRegionId();
	m_appliedRegionIds.push_back(regionId);

	setStatus("Affector applied successfully.");
	REPORT_LOG(true, ("[Titan] SwgCuiTerraforming::applyAffector - type=%d, pos=(%.1f, %.1f), radius=%ld\n",
		affectorType, centerX, centerZ, radius));
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::setRoadStart()
{
	float x, z;
	if (!validatePosition(x, z))
	{
		setStatus("Invalid position for road start.", true);
		return;
	}

	m_roadStartX = x;
	m_roadStartZ = z;
	m_hasRoadStart = true;

	if (m_textStartPos)
	{
		char buf[64];
		snprintf(buf, sizeof(buf), "(%.1f, %.1f)", x, z);
		m_textStartPos->SetLocalText(Unicode::narrowToWide(buf));
		m_textStartPos->SetTextColor(UIColor(0x66, 0xFF, 0x66));
	}

	updateRoadStatus();
	setStatus("Road start point set.");
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::setRoadEnd()
{
	float x, z;
	if (!validatePosition(x, z))
	{
		setStatus("Invalid position for road end.", true);
		return;
	}

	m_roadEndX = x;
	m_roadEndZ = z;
	m_hasRoadEnd = true;

	if (m_textEndPos)
	{
		char buf[64];
		snprintf(buf, sizeof(buf), "(%.1f, %.1f)", x, z);
		m_textEndPos->SetLocalText(Unicode::narrowToWide(buf));
		m_textEndPos->SetTextColor(UIColor(0x66, 0xFF, 0x66));
	}

	updateRoadStatus();
	setStatus("Road end point set.");
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::updateRoadStatus()
{
	bool canApply = m_hasRoadStart && m_hasRoadEnd;
	if (m_buttonRoadApply)
	{
		m_buttonRoadApply->SetEnabled(canApply);
	}

	if (m_textRoadStatus)
	{
		if (canApply)
		{
			float dx = m_roadEndX - m_roadStartX;
			float dz = m_roadEndZ - m_roadStartZ;
			float length = sqrtf(dx * dx + dz * dz);
			char buf[128];
			snprintf(buf, sizeof(buf), "Road length: %.1f meters. Ready to create.", length);
			m_textRoadStatus->SetLocalText(Unicode::narrowToWide(buf));
			m_textRoadStatus->SetTextColor(UIColor(0x00, 0xFF, 0x00));
		}
		else if (m_hasRoadStart)
		{
			m_textRoadStatus->SetLocalText(Unicode::narrowToWide("Start set. Now set the end point."));
			m_textRoadStatus->SetTextColor(UIColor(0xFF, 0xFF, 0x00));
		}
		else
		{
			m_textRoadStatus->SetLocalText(Unicode::narrowToWide("Set start and end points to create a road."));
			m_textRoadStatus->SetTextColor(UIColor(0xAA, 0xAA, 0xAA));
		}
	}
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::clearRoadMarkers()
{
	m_hasRoadStart = false;
	m_hasRoadEnd = false;
	m_roadStartX = 0.0f;
	m_roadStartZ = 0.0f;
	m_roadEndX = 0.0f;
	m_roadEndZ = 0.0f;

	if (m_textStartPos)
	{
		m_textStartPos->SetLocalText(Unicode::narrowToWide("Not Set"));
		m_textStartPos->SetTextColor(UIColor(0xFF, 0x66, 0x66));
	}
	if (m_textEndPos)
	{
		m_textEndPos->SetLocalText(Unicode::narrowToWide("Not Set"));
		m_textEndPos->SetTextColor(UIColor(0xFF, 0x66, 0x66));
	}

	updateRoadStatus();
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::refreshRegionList()
{
	m_regions.clear();

	REPORT_LOG(true, ("[Titan] SwgCuiTerraforming::refreshRegionList: cityId=%d\n", m_cityId));

	if (!m_listRegions)
	{
		REPORT_LOG(true, ("[Titan] SwgCuiTerraforming::refreshRegionList: m_listRegions is null\n"));
		return;
	}

	m_listRegions->Clear();

	if (!CityTerrainLayerManager::isInstalled())
	{
		REPORT_LOG(true, ("[Titan] SwgCuiTerraforming::refreshRegionList: CityTerrainLayerManager not installed\n"));
		return;
	}

	std::vector<const CityTerrainLayerManager::TerrainRegion *> cityRegions;
	CityTerrainLayerManager::getInstance().getRegionsForCity(m_cityId, cityRegions);

	REPORT_LOG(true, ("[Titan] SwgCuiTerraforming::refreshRegionList: found %d regions for city %d\n",
		static_cast<int>(cityRegions.size()), m_cityId));

	for (size_t i = 0; i < cityRegions.size(); ++i)
	{
		const CityTerrainLayerManager::TerrainRegion * region = cityRegions[i];
		if (!region)
			continue;

		RegionInfo info;
		info.regionId = region->regionId;
		info.centerX = region->centerX;
		info.centerZ = region->centerZ;
		info.radius = region->radius;

		switch (region->type)
		{
		case CityTerrainLayerManager::RT_SHADER_CIRCLE:
			info.type = "Shader Circle";
			info.displayName = "Shader: " + region->shaderTemplate;
			break;
		case CityTerrainLayerManager::RT_SHADER_LINE:
			info.type = "Road/Line";
			info.displayName = "Road: " + region->shaderTemplate;
			break;
		case CityTerrainLayerManager::RT_FLATTEN:
			info.type = "Height Mod";
			{
				char buf[64];
				snprintf(buf, sizeof(buf), "Flatten: height=%.1f", region->height);
				info.displayName = buf;
			}
			break;
		default:
			info.type = "Unknown";
			info.displayName = region->regionId;
			break;
		}

		m_regions.push_back(info);

		char rowText[256];
		snprintf(rowText, sizeof(rowText), "%s  |  (%.0f, %.0f)  |  r=%.0f",
			info.displayName.c_str(), info.centerX, info.centerZ, info.radius);

		m_listRegions->AddRow(Unicode::narrowToWide(rowText), info.regionId);
	}

	if (m_buttonRegionDelete)
		m_buttonRegionDelete->SetEnabled(false);

	REPORT_LOG(true, ("[Titan] SwgCuiTerraforming::refreshRegionList - found %d regions\n", static_cast<int>(m_regions.size())));
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::deleteSelectedRegion()
{
	if (!m_listRegions)
		return;

	int selectedRow = m_listRegions->GetLastSelectedRow();
	if (selectedRow < 0 || selectedRow >= static_cast<int>(m_regions.size()))
		return;

	std::string regionId = m_regions[selectedRow].regionId;

	sendTerrainRemove(regionId);

	if (CityTerrainLayerManager::isInstalled())
	{
		CityTerrainLayerManager::getInstance().removeRegion(regionId);
	}

	refreshRegionList();
	setStatus("Region deleted.");
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::deleteAllRegions()
{
	if (m_regions.empty())
	{
		setStatus("No regions to delete.");
		return;
	}

	for (size_t i = 0; i < m_regions.size(); ++i)
	{
		sendTerrainRemove(m_regions[i].regionId);
	}

	if (CityTerrainLayerManager::isInstalled())
	{
		CityTerrainLayerManager::getInstance().removeAllRegionsForCity(m_cityId);
	}

	refreshRegionList();
	setStatus("All regions deleted.");
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::showResetTerrainConfirmation()
{
	char message[512];
	snprintf(message, sizeof(message),
		"RESET TERRAIN - %s\n\n"
		"WARNING: This will reset ALL terrain within the city radius (%d meters) to the original planet terrain.\n\n"
		"All shader painting, height modifications, roads, and affectors will be permanently removed.\n\n"
		"This action cannot be undone.\n\n"
		"Are you sure you want to reset the terrain?",
		m_cityName.c_str(),
		m_cityRadius);

	CuiMessageBox * const box = CuiMessageBox::createYesNoBox(Unicode::narrowToWide(message));
	if (box)
	{
		m_callback->connect(box->getTransceiverClosed(), *this, &SwgCuiTerraforming::onResetTerrainConfirmationClosed);
	}
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::onResetTerrainConfirmationClosed(const CuiMessageBox & box)
{
	if (box.completedAffirmative())
	{
		resetTerrain();
	}
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::resetTerrain()
{
	if (m_cityId <= 0)
	{
		setStatus("Invalid city. Cannot reset terrain.", true);
		return;
	}

	CityTerrainPaintRequestMessage const msg(
		m_cityId,
		CityTerrainModificationType::MT_CLEAR_ALL,
		"",
		0.0f, 0.0f,
		static_cast<float>(m_cityRadius),
		0.0f, 0.0f,
		0.0f, 0.0f,
		0.0f
	);

	GameNetwork::send(msg, true);

	if (CityTerrainLayerManager::isInstalled())
	{
		CityTerrainLayerManager::getInstance().removeAllRegionsForCity(m_cityId);
	}

	m_regions.clear();
	m_appliedRegionIds.clear();
	refreshRegionList();

	setStatus("Terrain reset to original planet terrain.");
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::updateRegionInfo()
{
	if (!m_textRegionInfo || !m_listRegions)
		return;

	int selectedRow = m_listRegions->GetLastSelectedRow();
	if (selectedRow < 0 || selectedRow >= static_cast<int>(m_regions.size()))
	{
		m_textRegionInfo->SetLocalText(Unicode::narrowToWide("Select a region from the list above to see details."));
		return;
	}

	const RegionInfo & info = m_regions[selectedRow];
	char buf[512];
	snprintf(buf, sizeof(buf),
		"Type: %s\nLocation: (%.1f, %.1f)\nRadius: %.1f m\nID: %s",
		info.type.c_str(), info.centerX, info.centerZ, info.radius, info.regionId.c_str());

	m_textRegionInfo->SetLocalText(Unicode::narrowToWide(buf));
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::undoLastModification()
{
	if (m_appliedRegionIds.empty())
	{
		setStatus("Nothing to undo.");
		return;
	}

	std::string regionId = m_appliedRegionIds.back();
	m_appliedRegionIds.pop_back();

	sendTerrainRemove(regionId);

	if (CityTerrainLayerManager::isInstalled())
	{
		CityTerrainLayerManager::getInstance().removeRegion(regionId);
	}

	refreshRegionList();
	setStatus("Last modification undone.");
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::sampleCurrentHeight()
{
	Object const * const player = Game::getPlayer();
	if (!player)
		return;

	Vector const & pos = player->getPosition_w();
	float terrainHeight = pos.y;

	TerrainObject const * const terrain = TerrainObject::getInstance();
	if (terrain)
	{
		float height = 0.0f;
		if (terrain->getHeight(pos, height))
		{
			terrainHeight = height;
		}
	}

	if (m_sliderHeightValue)
	{
		long heightLong = static_cast<long>(terrainHeight);
		long lower = m_sliderHeightValue->GetLowerLimit();
		long upper = m_sliderHeightValue->GetUpperLimit();
		if (heightLong < lower) heightLong = lower;
		if (heightLong > upper) heightLong = upper;
		m_sliderHeightValue->SetValue(heightLong, true);
	}

	if (m_textHeightValue)
	{
		char buf[32];
		snprintf(buf, sizeof(buf), "%d", static_cast<int>(terrainHeight));
		m_textHeightValue->SetLocalText(Unicode::narrowToWide(buf));
	}

	setStatus("Height sampled from current position.");
}

// ----------------------------------------------------------------------

bool SwgCuiTerraforming::validateCityPermission() const
{
	return m_cityId != 0;
}

// ----------------------------------------------------------------------

bool SwgCuiTerraforming::validatePosition(float & outX, float & outZ) const
{
	Object const * const player = Game::getPlayer();
	if (!player)
		return false;

	Vector const & pos = player->getPosition_w();
	outX = pos.x;
	outZ = pos.z;
	return true;
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::setStatus(const std::string & message, bool isError)
{
	if (!m_textStatus)
		return;

	m_textStatus->SetLocalText(Unicode::narrowToWide(message));
	m_textStatus->SetTextColor(isError ? UIColor(0xFF, 0x66, 0x66) : UIColor(0x00, 0xFF, 0x00));
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::clearStatus()
{
	if (m_textStatus)
	{
		m_textStatus->SetLocalText(Unicode::emptyString);
	}
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::saveUIState()
{
	ms_savedCityId = m_cityId;
	ms_savedTab = m_currentTab;

	if (m_comboShader)
		ms_savedShaderIndex = m_comboShader->GetSelectedIndex();
	if (m_sliderPaintRadius)
		ms_savedPaintRadius = m_sliderPaintRadius->GetValue();
	if (m_sliderPaintBlend)
		ms_savedPaintBlend = m_sliderPaintBlend->GetValue();
	if (m_sliderPaintFeather)
		ms_savedPaintFeather = m_sliderPaintFeather->GetValue();

	if (m_comboHeightMode)
		ms_savedHeightMode = m_comboHeightMode->GetSelectedIndex();
	if (m_sliderHeightValue)
		ms_savedHeightValue = m_sliderHeightValue->GetValue();
	if (m_sliderHeightRadius)
		ms_savedHeightRadius = m_sliderHeightRadius->GetValue();
	if (m_sliderHeightStrength)
		ms_savedHeightStrength = m_sliderHeightStrength->GetValue();
	if (m_sliderHeightFeather)
		ms_savedHeightFeather = m_sliderHeightFeather->GetValue();

	if (m_sliderRoadWidth)
		ms_savedRoadWidth = m_sliderRoadWidth->GetValue();
	if (m_comboRoadShader)
		ms_savedRoadShaderIndex = m_comboRoadShader->GetSelectedIndex();

	if (m_comboAffectorType)
		ms_savedAffectorType = m_comboAffectorType->GetSelectedIndex();
	if (m_sliderAffectorRadius)
		ms_savedAffectorRadius = m_sliderAffectorRadius->GetValue();
	if (m_sliderAffectorIntensity)
		ms_savedAffectorIntensity = m_sliderAffectorIntensity->GetValue();
	if (m_sliderAffectorFeather)
		ms_savedAffectorFeather = m_sliderAffectorFeather->GetValue();

	ms_hasRestoredState = true;
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::restoreUIState()
{
	if (!ms_hasRestoredState || ms_savedCityId != m_cityId)
		return;

	m_currentTab = ms_savedTab;

	if (m_comboShader && ms_savedShaderIndex >= 0)
		m_comboShader->SetSelectedIndex(ms_savedShaderIndex);
	if (m_sliderPaintRadius)
	{
		m_sliderPaintRadius->SetValue(ms_savedPaintRadius, false);
		syncSliderToText(m_sliderPaintRadius, m_textPaintRadius);
	}
	if (m_sliderPaintBlend)
	{
		m_sliderPaintBlend->SetValue(ms_savedPaintBlend, false);
		syncSliderToText(m_sliderPaintBlend, m_textPaintBlend);
	}
	if (m_sliderPaintFeather)
	{
		m_sliderPaintFeather->SetValue(ms_savedPaintFeather, false);
		syncSliderToText(m_sliderPaintFeather, m_textPaintFeather);
	}

	if (m_comboHeightMode && ms_savedHeightMode >= 0)
		m_comboHeightMode->SetSelectedIndex(ms_savedHeightMode);
	if (m_sliderHeightValue)
	{
		m_sliderHeightValue->SetValue(ms_savedHeightValue, false);
		syncSliderToText(m_sliderHeightValue, m_textHeightValue);
	}
	if (m_sliderHeightRadius)
	{
		m_sliderHeightRadius->SetValue(ms_savedHeightRadius, false);
		syncSliderToText(m_sliderHeightRadius, m_textHeightRadius);
	}
	if (m_sliderHeightStrength)
	{
		m_sliderHeightStrength->SetValue(ms_savedHeightStrength, false);
		syncSliderToText(m_sliderHeightStrength, m_textHeightStrength);
	}
	if (m_sliderHeightFeather)
	{
		m_sliderHeightFeather->SetValue(ms_savedHeightFeather, false);
		syncSliderToText(m_sliderHeightFeather, m_textHeightFeather);
	}

	if (m_sliderRoadWidth)
	{
		m_sliderRoadWidth->SetValue(ms_savedRoadWidth, false);
		syncSliderToText(m_sliderRoadWidth, m_textRoadWidth);
	}
	if (m_comboRoadShader && ms_savedRoadShaderIndex >= 0)
		m_comboRoadShader->SetSelectedIndex(ms_savedRoadShaderIndex);

	if (m_comboAffectorType && ms_savedAffectorType >= 0)
		m_comboAffectorType->SetSelectedIndex(ms_savedAffectorType);
	if (m_sliderAffectorRadius)
	{
		m_sliderAffectorRadius->SetValue(ms_savedAffectorRadius, false);
		syncSliderToText(m_sliderAffectorRadius, m_textAffectorRadius);
	}
	if (m_sliderAffectorIntensity)
	{
		m_sliderAffectorIntensity->SetValue(ms_savedAffectorIntensity, false);
		syncSliderToText(m_sliderAffectorIntensity, m_textAffectorIntensity);
	}
	if (m_sliderAffectorFeather)
	{
		m_sliderAffectorFeather->SetValue(ms_savedAffectorFeather, false);
		syncSliderToText(m_sliderAffectorFeather, m_textAffectorFeather);
	}
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::sendTerrainModification(int32 modType, const std::string & shaderTemplate,
	float centerX, float centerZ, float radius, float endX, float endZ,
	float width, float height, float blendDistance)
{
	CityTerrainPaintRequestMessage msg(
		m_cityId,
		modType,
		shaderTemplate,
		centerX,
		centerZ,
		radius,
		endX,
		endZ,
		width,
		height,
		blendDistance
	);

	GameNetwork::send(msg, true);
}

// ----------------------------------------------------------------------

void SwgCuiTerraforming::sendTerrainRemove(const std::string & regionId)
{
	CityTerrainRemoveRequestMessage msg(m_cityId, regionId);
	GameNetwork::send(msg, true);
}

// ----------------------------------------------------------------------

std::string SwgCuiTerraforming::generateRegionId() const
{
	char buf[64];
	unsigned int seed = static_cast<unsigned int>(time(nullptr)) ^ static_cast<unsigned int>(rand());
	snprintf(buf, sizeof(buf), "city%d_region_%08x", m_cityId, seed);
	return std::string(buf);
}

// ======================================================================




