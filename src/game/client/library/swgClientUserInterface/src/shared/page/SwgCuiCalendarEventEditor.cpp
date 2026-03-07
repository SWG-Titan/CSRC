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

#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "sharedNetworkMessages/CalendarMessages.h"

#include "UIButton.h"
#include "UICheckbox.h"
#include "UIComboBox.h"
#include "UIData.h"
#include "UIPage.h"
#include "UIText.h"
#include "UITextbox.h"
#include "Unicode.h"

#include <ctime>

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
m_buttonCreate       (0),
m_buttonCancel       (0)
{
	getCodeDataObject(TUITextbox,  m_textTitle,        "titleSection.txtTitle", true);
	getCodeDataObject(TUITextbox,  m_textDescription,  "descSection.txtDescription", true);
	getCodeDataObject(TUIComboBox, m_comboEventType,   "typeSection.cmbEventType", true);
	getCodeDataObject(TUITextbox,  m_textDate,         "dateSection.txtDate", true);
	getCodeDataObject(TUITextbox,  m_textTime,         "dateSection.txtTime", true);
	getCodeDataObject(TUIComboBox, m_comboDuration,    "durationSection.cmbDuration", true);
	getCodeDataObject(TUIPage,     m_pageServerEvent,  "serverEventSection", true);
	getCodeDataObject(TUIComboBox, m_comboServerEvent, "serverEventSection.cmbServerEvent", true);
	getCodeDataObject(TUICheckbox, m_checkBroadcast,   "optionsSection.chkBroadcast", true);
	getCodeDataObject(TUICheckbox, m_checkRecurring,   "optionsSection.chkRecurring", true);
	getCodeDataObject(TUIPage,     m_pageRecurrence,   "recurrenceSection", true);
	getCodeDataObject(TUIComboBox, m_comboRecurrence,  "recurrenceSection.cmbRecurrence", true);
	getCodeDataObject(TUIButton,   m_buttonCreate,     "btnCreateEvent", true);
	getCodeDataObject(TUIButton,   m_buttonCancel,     "btnCancel", true);

	time_t now = time(0);
	struct tm * timeinfo = localtime(&now);
	if (timeinfo && m_textDate && m_textTime)
	{
		char dateBuffer[32];
		char timeBuffer[16];
		snprintf(dateBuffer, sizeof(dateBuffer), "%04d-%02d-%02d",
			timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);
		snprintf(timeBuffer, sizeof(timeBuffer), "12:00");

		m_textDate->SetText(Unicode::narrowToWide(dateBuffer));
		m_textTime->SetText(Unicode::narrowToWide(timeBuffer));
	}
}

//----------------------------------------------------------------------

SwgCuiCalendarEventEditor::~SwgCuiCalendarEventEditor()
{
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
	// Gather form data
	Unicode::String titleW;
	Unicode::String descW;
	Unicode::String dateW;
	Unicode::String timeW;

	if (m_textTitle)       m_textTitle->GetText(titleW);
	if (m_textDescription) m_textDescription->GetText(descW);
	if (m_textDate)        m_textDate->GetText(dateW);
	if (m_textTime)        m_textTime->GetText(timeW);

	std::string title = Unicode::wideToNarrow(titleW);
	std::string desc = Unicode::wideToNarrow(descW);
	std::string date = Unicode::wideToNarrow(dateW);
	std::string timeStr = Unicode::wideToNarrow(timeW);

	// Validate title
	if (title.empty())
	{
		return;
	}

	int eventType = m_comboEventType ? m_comboEventType->GetSelectedIndex() : 0;
	int durationIdx = m_comboDuration ? m_comboDuration->GetSelectedIndex() : 2;
	int recurrenceIdx = m_comboRecurrence ? m_comboRecurrence->GetSelectedIndex() : 0;
	bool broadcast = m_checkBroadcast ? m_checkBroadcast->IsChecked() : true;
	bool recurring = m_checkRecurring ? m_checkRecurring->IsChecked() : false;

	// Parse date (YYYY-MM-DD)
	int year = 2026, month = 1, day = 1;
	if (sscanf(date.c_str(), "%d-%d-%d", &year, &month, &day) != 3)
	{
		return;
	}

	// Parse time (HH:MM)
	int hour = 12, minute = 0;
	if (sscanf(timeStr.c_str(), "%d:%d", &hour, &minute) != 2)
	{
		hour = 12;
		minute = 0;
	}

	// Map duration index to minutes
	static int const durations[] = {15, 30, 60, 120, 240, 480, 1440};
	int duration = (durationIdx >= 0 && durationIdx < 7) ? durations[durationIdx] : 60;

	// Build event data
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

	// Server event key
	if (eventType == 3 && m_comboServerEvent)
	{
		int serverEventIdx = m_comboServerEvent->GetSelectedIndex();
		static char const * const serverEventKeys[] = {"halloween", "lifeday", "loveday", "empireday_ceremony"};
		if (serverEventIdx >= 0 && serverEventIdx < 4)
		{
			eventData.serverEventKey = serverEventKeys[serverEventIdx];
		}
	}

	// Send create message to server
	CalendarCreateEventMessage msg(eventData);
	GameNetwork::send(msg, true);

	// Close the editor
	deactivate();
}

//======================================================================
