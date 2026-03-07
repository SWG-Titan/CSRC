//======================================================================
//
// SwgCuiCalendarEventEditor.cpp
// copyright (c) 2026 Titan Project
//
// In-Game Calendar System - Event Editor Dialog
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCalendarEventEditor.h"
#include "swgClientUserInterface/SwgCuiCalendar.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"

#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedNetworkMessages/CalendarMessages.h"

#include "UIButton.h"
#include "UICheckbox.h"
#include "UIComboBox.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIPage.h"
#include "UIText.h"
#include "UITextbox.h"
#include "Unicode.h"

#include <ctime>

//======================================================================

namespace SwgCuiCalendarEventEditorNamespace
{
	char const * const s_loadingTextureCandidates[] =
	{
		"texture/loading/corellia.dds",
		"texture/loading/dantooine.dds",
		"texture/loading/dathomir.dds",
		"texture/loading/endor.dds",
		"texture/loading/kashyyyk_main.dds",
		"texture/loading/lok.dds",
		"texture/loading/naboo.dds",
		"texture/loading/rori.dds",
		"texture/loading/talus.dds",
		"texture/loading/tatooine.dds",
		"texture/loading/yavin4.dds",
		"texture/loading/mustafar.dds",
		"texture/loading/hoth.dds",
		"texture/loading/ord_mantell.dds",
		"texture/loading/dungeon.dds",
		"texture/loading/tutorial.dds",
		"texture/loading/space_corellia.dds",
		"texture/loading/space_dantooine.dds",
		"texture/loading/space_dathomir.dds",
		"texture/loading/space_endor.dds",
		"texture/loading/space_kashyyyk.dds",
		"texture/loading/space_lok.dds",
		"texture/loading/space_naboo.dds",
		"texture/loading/space_tatooine.dds",
		"texture/loading/space_yavin4.dds",
		"texture/loading/space_deep.dds",
		"texture/loading/space_kessel.dds",
		"texture/loading/space_nova_orion.dds",
		"texture/loading/large/large_load_watto.dds",
		"texture/loading/large/large_load_corellian_corvette.dds",
		"texture/loading/large/large_load_dawn.dds",
		"texture/loading/large/large_load_desert.dds",
		"texture/loading/large/large_load_forest.dds",
		"texture/loading/large/large_load_imperial.dds",
		"texture/loading/large/large_load_jabba.dds",
		"texture/loading/large/large_load_jedi.dds",
		"texture/loading/large/large_load_mos_eisley.dds",
		"texture/loading/large/large_load_rebel.dds",
		"texture/loading/large/large_load_space.dds",
		"texture/loading/large/large_load_starport.dds",
		"texture/loading/large/large_load_swamp.dds",
		"texture/loading/large/large_load_theed.dds",
		"texture/loading/large/large_load_cantina.dds",
		"texture/loading/large/large_load_city.dds",
		"texture/loading/large/large_load_coronet.dds",
		"texture/loading/large/large_load_deathstar.dds",
		"texture/loading/large/large_load_guild.dds",
		"texture/loading/large/large_load_hunt.dds",
		"texture/loading/large/large_load_kashyyyk.dds",
		"texture/loading/large/large_load_mining.dds",
		"texture/loading/large/large_load_mustafar.dds",
		"texture/loading/large/large_load_nightsisters.dds",
		"texture/loading/large/large_load_pod_race.dds",
		"texture/loading/large/large_load_snowspeeder.dds",
		"texture/loading/large/large_load_tie.dds",
		"texture/loading/large/large_load_xwing.dds",
	};

	int const s_numLoadingTextureCandidates = sizeof(s_loadingTextureCandidates) / sizeof(s_loadingTextureCandidates[0]);
}

using namespace SwgCuiCalendarEventEditorNamespace;

//======================================================================

SwgCuiCalendarEventEditor * SwgCuiCalendarEventEditor::ms_instance = 0;

//======================================================================

SwgCuiCalendarEventEditor::SwgCuiCalendarEventEditor(UIPage & page) :
CuiMediator          ("SwgCuiCalendarEventEditor", page),
UIEventCallback      (),
m_textTitle          (0),
m_textDescription    (0),
m_comboEventType     (0),
m_textDate           (0),
m_textTime           (0),
m_comboDuration      (0),
m_pageServerEvent    (0),
m_comboServerEvent   (0),
m_checkBroadcast     (0),
m_checkRecurring     (0),
m_pageRecurrence     (0),
m_comboRecurrence    (0),
m_comboImage         (0),
m_buttonCreate       (0),
m_buttonCancel       (0),
m_editingEvent       (),
m_isEditing          (false)
{
	getCodeDataObject(TUITextbox,  m_textTitle,        "titleSection.txtTitle", false);
	getCodeDataObject(TUITextbox,  m_textDescription,  "descSection.txtDescription", false);
	getCodeDataObject(TUIComboBox, m_comboEventType,   "typeSection.cmbEventType", false);
	getCodeDataObject(TUITextbox,  m_textDate,         "dateSection.txtDate", false);
	getCodeDataObject(TUITextbox,  m_textTime,         "dateSection.txtTime", false);
	getCodeDataObject(TUIComboBox, m_comboDuration,    "durationSection.cmbDuration", false);
	getCodeDataObject(TUIPage,     m_pageServerEvent,  "serverEventSection", false);
	getCodeDataObject(TUIComboBox, m_comboServerEvent, "serverEventSection.cmbServerEvent", false);
	getCodeDataObject(TUICheckbox, m_checkBroadcast,   "optionsSection.chkBroadcast", false);
	getCodeDataObject(TUICheckbox, m_checkRecurring,   "optionsSection.chkRecurring", false);
	getCodeDataObject(TUIPage,     m_pageRecurrence,   "recurrenceSection", false);
	getCodeDataObject(TUIComboBox, m_comboRecurrence,  "recurrenceSection.cmbRecurrence", false);
	getCodeDataObject(TUIComboBox, m_comboImage,       "imageSection.cmbImage", false);
	getCodeDataObject(TUIButton,   m_buttonCreate,     "btnCreateEvent", false);
	getCodeDataObject(TUIButton,   m_buttonCancel,     "btnCancel", false);

	ms_instance = this;
}

//----------------------------------------------------------------------

SwgCuiCalendarEventEditor::~SwgCuiCalendarEventEditor()
{
	ms_instance = 0;

	m_textTitle        = 0;
	m_textDescription  = 0;
	m_comboEventType   = 0;
	m_textDate         = 0;
	m_textTime         = 0;
	m_comboDuration    = 0;
	m_pageServerEvent  = 0;
	m_comboServerEvent = 0;
	m_checkBroadcast   = 0;
	m_checkRecurring   = 0;
	m_pageRecurrence   = 0;
	m_comboRecurrence  = 0;
	m_comboImage       = 0;
	m_buttonCreate     = 0;
	m_buttonCancel     = 0;
}

//----------------------------------------------------------------------

void SwgCuiCalendarEventEditor::performActivate()
{
	if (m_buttonCreate)    m_buttonCreate->AddCallback(this);
	if (m_buttonCancel)    m_buttonCancel->AddCallback(this);
	if (m_checkRecurring)  m_checkRecurring->AddCallback(this);
	if (m_comboEventType)  m_comboEventType->AddCallback(this);

	// Populate image dropdown dynamically from the TRE archive
	if (m_comboImage)
	{
		// Create a new DataSource for the combobox if it doesn't have one
		if (!m_comboImage->GetDataSource())
		{
			UIDataSource * const dataSource = new UIDataSource;
			dataSource->SetName("dsImages");
			m_comboImage->SetDataSource(dataSource);
			REPORT_LOG(true, ("SwgCuiCalendarEventEditor: Created new DataSource for image combobox\n"));
		}

		m_comboImage->Clear();
		m_comboImage->AddItem(Unicode::narrowToWide("None"), "none");
		REPORT_LOG(true, ("SwgCuiCalendarEventEditor: Adding 'None' to image dropdown\n"));

		for (int i = 0; i < s_numLoadingTextureCandidates; ++i)
		{
			m_comboImage->AddItem(
				Unicode::narrowToWide(s_loadingTextureCandidates[i]),
				s_loadingTextureCandidates[i]);
			REPORT_LOG(true, ("SwgCuiCalendarEventEditor: Added image [%d]: %s\n", i, s_loadingTextureCandidates[i]));
		}

		REPORT_LOG(true, ("SwgCuiCalendarEventEditor: Total images added: %d, ItemCount: %d\n", s_numLoadingTextureCandidates + 1, m_comboImage->GetItemCount()));
		m_comboImage->SetSelectedIndex(0);
	}
	else
	{
		REPORT_LOG(true, ("SwgCuiCalendarEventEditor: m_comboImage is NULL!\n"));
	}

	// If editing, populate from event data
	if (m_isEditing)
	{
		populateFromEventData();
		if (m_buttonCreate)
			m_buttonCreate->SetText(Unicode::narrowToWide("Save Changes"));
	}
	else
	{
		// Pre-fill date from the calendar's selected date
		int year  = SwgCuiCalendar::getSelectedYear();
		int month = SwgCuiCalendar::getSelectedMonth();
		int day   = SwgCuiCalendar::getSelectedDay();

		if (m_textDate)
		{
			char dateBuffer[32];
			snprintf(dateBuffer, sizeof(dateBuffer), "%04d-%02d-%02d", year, month, day);
			m_textDate->SetText(Unicode::narrowToWide(dateBuffer));
		}

		if (m_textTime)
		{
			m_textTime->SetText(Unicode::narrowToWide("12:00"));
		}

		// Clear title and description for a fresh form
		if (m_textTitle)
			m_textTitle->SetText(Unicode::emptyString);
		if (m_textDescription)
			m_textDescription->SetText(Unicode::emptyString);

		if (m_buttonCreate)
			m_buttonCreate->SetText(Unicode::narrowToWide("Create Event"));
	}

	updateRecurrenceVisibility();
	updateServerEventVisibility();
}

//----------------------------------------------------------------------

void SwgCuiCalendarEventEditor::performDeactivate()
{
	if (m_buttonCreate)    m_buttonCreate->RemoveCallback(this);
	if (m_buttonCancel)    m_buttonCancel->RemoveCallback(this);
	if (m_checkRecurring)  m_checkRecurring->RemoveCallback(this);
	if (m_comboEventType)  m_comboEventType->RemoveCallback(this);

	// Clear editing state when closing
	clearEventData();
}

//----------------------------------------------------------------------

void SwgCuiCalendarEventEditor::OnButtonPressed(UIWidget *context)
{
	if (context == m_buttonCreate)
	{
		createEvent();
	}
	else if (context == m_buttonCancel)
	{
		deactivate();
	}
}

//----------------------------------------------------------------------

void SwgCuiCalendarEventEditor::OnCheckboxSet(UIWidget *context)
{
	if (context == m_checkRecurring)
	{
		updateRecurrenceVisibility();
	}
}

//----------------------------------------------------------------------

void SwgCuiCalendarEventEditor::OnCheckboxUnset(UIWidget *context)
{
	if (context == m_checkRecurring)
	{
		updateRecurrenceVisibility();
	}
}

//----------------------------------------------------------------------

void SwgCuiCalendarEventEditor::OnGenericSelectionChanged(UIWidget *context)
{
	if (context == m_comboEventType)
	{
		updateServerEventVisibility();
	}
}

//----------------------------------------------------------------------

void SwgCuiCalendarEventEditor::updateRecurrenceVisibility()
{
	if (m_pageRecurrence && m_checkRecurring)
	{
		m_pageRecurrence->SetVisible(m_checkRecurring->IsChecked());
	}
}

//----------------------------------------------------------------------

void SwgCuiCalendarEventEditor::updateServerEventVisibility()
{
	if (m_pageServerEvent && m_comboEventType)
	{
		int selectedIndex = m_comboEventType->GetSelectedIndex();
		m_pageServerEvent->SetVisible(selectedIndex == 3);
	}
}

//----------------------------------------------------------------------

void SwgCuiCalendarEventEditor::setDefaultDate(int year, int month, int day)
{
	if (m_textDate)
	{
		char buffer[32];
		snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d", year, month, day);
		m_textDate->SetText(Unicode::narrowToWide(buffer));
	}
}

//----------------------------------------------------------------------

void SwgCuiCalendarEventEditor::createEvent()
{
	Unicode::String titleW;
	Unicode::String descW;
	Unicode::String dateW;
	Unicode::String timeW;

	if (m_textTitle)       titleW = m_textTitle->GetLocalText();
	if (m_textDescription) descW = m_textDescription->GetLocalText();
	if (m_textDate)        dateW = m_textDate->GetLocalText();
	if (m_textTime)        timeW = m_textTime->GetLocalText();

	std::string title = Unicode::wideToNarrow(titleW);
	std::string desc = Unicode::wideToNarrow(descW);
	std::string date = Unicode::wideToNarrow(dateW);
	std::string timeStr = Unicode::wideToNarrow(timeW);

	if (title.empty())
	{
		REPORT_LOG(true, ("SwgCuiCalendarEventEditor::createEvent: title is empty, aborting\n"));
		return;
	}

	int eventType = m_comboEventType ? m_comboEventType->GetSelectedIndex() : 0;
	int durationIdx = m_comboDuration ? m_comboDuration->GetSelectedIndex() : 2;
	int recurrenceIdx = m_comboRecurrence ? m_comboRecurrence->GetSelectedIndex() : 0;
	bool broadcast = m_checkBroadcast ? m_checkBroadcast->IsChecked() : true;
	bool recurring = m_checkRecurring ? m_checkRecurring->IsChecked() : false;

	REPORT_LOG(true, ("SwgCuiCalendarEventEditor::createEvent: title=[%s] date=[%s] time=[%s] type=%d durationIdx=%d\n",
		title.c_str(), date.c_str(), timeStr.c_str(), eventType, durationIdx));

	int year = 2026, month = 1, day = 1;
	if (sscanf(date.c_str(), "%d-%d-%d", &year, &month, &day) != 3)
	{
		REPORT_LOG(true, ("SwgCuiCalendarEventEditor::createEvent: failed to parse date [%s], aborting\n", date.c_str()));
		return;
	}

	int hour = 12, minute = 0;
	if (sscanf(timeStr.c_str(), "%d:%d", &hour, &minute) != 2)
	{
		hour = 12;
		minute = 0;
	}

	static int const durations[] = {15, 30, 60, 120, 240, 480, 1440};
	int duration = (durationIdx >= 0 && durationIdx < 7) ? durations[durationIdx] : 60;

	CalendarEventData eventData;
	eventData.title = title;
	eventData.description = desc;
	eventData.eventType = eventType;
	eventData.year = year;
	eventData.month = month;
	eventData.day = day;
	eventData.hour = hour;
	eventData.minute = minute;
	eventData.duration = duration;
	eventData.broadcastStart = broadcast;
	eventData.recurring = recurring;
	eventData.recurrenceType = recurring ? recurrenceIdx + 1 : 0;
	eventData.guildId = 0;
	eventData.cityId = 0;
	eventData.active = false;

	if (eventType == 3 && m_comboServerEvent)
	{
		int serverEventIdx = m_comboServerEvent->GetSelectedIndex();
		static char const * const serverEventKeys[] = {"halloween", "lifeday", "loveday", "empireday_ceremony"};
		if (serverEventIdx >= 0 && serverEventIdx < 4)
		{
			eventData.serverEventKey = serverEventKeys[serverEventIdx];
		}
	}

	if (GameNetwork::isConnectedToConnectionServer())
	{
		REPORT_LOG(true, ("SwgCuiCalendarEventEditor::createEvent: SENDING CalendarCreateEventMessage title=[%s] type=%d date=%d-%02d-%02d %02d:%02d duration=%d\n",
			eventData.title.c_str(), eventData.eventType, eventData.year, eventData.month, eventData.day, eventData.hour, eventData.minute, eventData.duration));
		CalendarCreateEventMessage msg(eventData);
		GameNetwork::send(msg, true);
	}
	else
	{
		REPORT_LOG(true, ("SwgCuiCalendarEventEditor::createEvent: NOT connected, cannot send\n"));
	}

	deactivate();
}

//----------------------------------------------------------------------

void SwgCuiCalendarEventEditor::setEventData(CalendarEventData const & eventData)
{
	m_editingEvent = eventData;
	m_isEditing = true;
}

//----------------------------------------------------------------------

void SwgCuiCalendarEventEditor::clearEventData()
{
	m_editingEvent = CalendarEventData();
	m_isEditing = false;
}

//----------------------------------------------------------------------

void SwgCuiCalendarEventEditor::populateFromEventData()
{
	if (m_textTitle)
		m_textTitle->SetText(Unicode::narrowToWide(m_editingEvent.title));

	if (m_textDescription)
		m_textDescription->SetText(Unicode::narrowToWide(m_editingEvent.description));

	if (m_comboEventType)
	{
		int typeIdx = m_editingEvent.eventType;
		if (typeIdx >= 0 && typeIdx < 4)
			m_comboEventType->SetSelectedIndex(typeIdx);
	}

	if (m_textDate)
	{
		char dateBuffer[32];
		snprintf(dateBuffer, sizeof(dateBuffer), "%04d-%02d-%02d",
			m_editingEvent.year, m_editingEvent.month, m_editingEvent.day);
		m_textDate->SetText(Unicode::narrowToWide(dateBuffer));
	}

	if (m_textTime)
	{
		char timeBuffer[16];
		snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d",
			m_editingEvent.hour, m_editingEvent.minute);
		m_textTime->SetText(Unicode::narrowToWide(timeBuffer));
	}

	if (m_comboDuration)
	{
		static int const durations[] = {15, 30, 60, 120, 240, 480, 1440};
		int durationIdx = 2;
		for (int i = 0; i < 7; ++i)
		{
			if (durations[i] == m_editingEvent.duration)
			{
				durationIdx = i;
				break;
			}
		}
		m_comboDuration->SetSelectedIndex(durationIdx);
	}

	if (m_checkBroadcast)
		m_checkBroadcast->SetChecked(m_editingEvent.broadcastStart);

	if (m_checkRecurring)
		m_checkRecurring->SetChecked(m_editingEvent.recurring);

	if (m_comboRecurrence && m_editingEvent.recurrenceType > 0)
		m_comboRecurrence->SetSelectedIndex(m_editingEvent.recurrenceType - 1);

	updateRecurrenceVisibility();
	updateServerEventVisibility();
}

//----------------------------------------------------------------------

SwgCuiCalendarEventEditor * SwgCuiCalendarEventEditor::getInstance()
{
	return ms_instance;
}

//----------------------------------------------------------------------

void SwgCuiCalendarEventEditor::editEvent(CalendarEventData const & eventData)
{
	CuiMediatorFactory::activateInWorkspace(CuiMediatorTypes::WS_CalendarEventEditor);
	if (ms_instance)
	{
		ms_instance->setEventData(eventData);
	}
}

//======================================================================
