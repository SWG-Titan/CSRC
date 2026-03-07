//======================================================================
//
// SwgCuiCalendar.cpp
// copyright (c) 2026 Titan Project
//
// In-Game Calendar System - Main Calendar Window
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCalendar.h"

#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/CalendarMessages.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIList.h"
#include "UIPage.h"
#include "UIText.h"
#include "Unicode.h"

#include <ctime>

//======================================================================

namespace
{
	const char * const s_monthNames[] = {
		"January", "February", "March", "April", "May", "June",
		"July", "August", "September", "October", "November", "December"
	};

	int getDaysInMonth(int year, int month)
	{
		static const int daysPerMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
		if (month < 1 || month > 12)
			return 30;

		int days = daysPerMonth[month - 1];

		if (month == 2)
		{
			bool isLeap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
			if (isLeap)
				days = 29;
		}

		return days;
	}

	int getFirstDayOfMonth(int year, int month)
	{
		if (month < 3)
		{
			month += 12;
			year--;
		}

		int k = year % 100;
		int j = year / 100;
		int dayOfWeek = (1 + (13 * (month + 1)) / 5 + k + k / 4 + j / 4 + 5 * j) % 7;
		dayOfWeek = ((dayOfWeek + 6) % 7);

		return dayOfWeek + 1;
	}
}

//======================================================================

SwgCuiCalendar::SwgCuiCalendar(UIPage & page) :
CuiMediator         ("SwgCuiCalendar", page),
UIEventCallback     (),
MessageDispatch::Receiver(),
m_buttonPrevMonth   (0),
m_buttonNextMonth   (0),
m_buttonToday       (0),
m_buttonCreate      (0),
m_buttonSettings    (0),
m_buttonClose       (0),
m_textMonthYear     (0),
m_textSelectedDate  (0),
m_listEvents        (0),
m_dataEvents        (0),
m_buttonViewDetails (0),
m_buttonDeleteEvent (0),
m_currentYear       (2026),
m_currentMonth      (3),
m_selectedDay       (6),
m_cachedEvents      (),
m_callback          (0)
{
	for (int i = 0; i < 42; ++i)
	{
		m_dayButtons[i] = 0;
	}

	getCodeDataObject(TUIButton, m_buttonPrevMonth,   "header.btnPrevMonth", true);
	getCodeDataObject(TUIButton, m_buttonNextMonth,   "header.btnNextMonth", true);
	getCodeDataObject(TUIButton, m_buttonToday,       "header.btnToday", true);
	getCodeDataObject(TUIButton, m_buttonCreate,      "header.btnCreate", true);
	getCodeDataObject(TUIButton, m_buttonSettings,    "header.btnSettings", true);
	getCodeDataObject(TUIButton, m_buttonClose,       "btnClose", true);
	getCodeDataObject(TUIText,   m_textMonthYear,     "header.lblMonthYear", true);

	getCodeDataObject(TUIText,       m_textSelectedDate,  "eventPanel.lblSelectedDate", true);
	getCodeDataObject(TUIList,       m_listEvents,        "eventPanel.lstEvents", true);
	getCodeDataObject(TUIDataSource, m_dataEvents,        "eventPanel.dataEvents", true);
	getCodeDataObject(TUIButton,     m_buttonViewDetails, "eventPanel.btnViewDetails", true);
	getCodeDataObject(TUIButton,     m_buttonDeleteEvent, "eventPanel.btnDeleteEvent", true);

	char buttonName[64];
	for (int i = 0; i < 42; ++i)
	{
		int dayNum = i + 1;
		snprintf(buttonName, sizeof(buttonName), "calendarGrid.day%02d", dayNum);
		getCodeDataObject(TUIButton, m_dayButtons[i], buttonName, true);
	}

	time_t now = time(0);
	struct tm * timeinfo = localtime(&now);
	if (timeinfo)
	{
		m_currentYear = timeinfo->tm_year + 1900;
		m_currentMonth = timeinfo->tm_mon + 1;
		m_selectedDay = timeinfo->tm_mday;
	}
}

//----------------------------------------------------------------------

SwgCuiCalendar::~SwgCuiCalendar()
{
	delete m_callback;
	m_callback = 0;

	m_buttonPrevMonth   = 0;
	m_buttonNextMonth   = 0;
	m_buttonToday       = 0;
	m_buttonCreate      = 0;
	m_buttonSettings    = 0;
	m_buttonClose       = 0;
	m_textMonthYear     = 0;
	m_textSelectedDate  = 0;
	m_listEvents        = 0;
	m_dataEvents        = 0;
	m_buttonViewDetails = 0;
	m_buttonDeleteEvent = 0;

	for (int i = 0; i < 42; ++i)
	{
		m_dayButtons[i] = 0;
	}
}

//----------------------------------------------------------------------

void SwgCuiCalendar::performActivate()
{
	if (m_buttonPrevMonth)   m_buttonPrevMonth->AddCallback(this);
	if (m_buttonNextMonth)   m_buttonNextMonth->AddCallback(this);
	if (m_buttonToday)       m_buttonToday->AddCallback(this);
	if (m_buttonCreate)      m_buttonCreate->AddCallback(this);
	if (m_buttonSettings)    m_buttonSettings->AddCallback(this);
	if (m_buttonClose)       m_buttonClose->AddCallback(this);
	if (m_buttonViewDetails) m_buttonViewDetails->AddCallback(this);
	if (m_buttonDeleteEvent) m_buttonDeleteEvent->AddCallback(this);

	for (int i = 0; i < 42; ++i)
	{
		if (m_dayButtons[i])
			m_dayButtons[i]->AddCallback(this);
	}

	if (m_listEvents)
		m_listEvents->AddCallback(this);

	// Request events from server
	requestEvents();

	updateCalendarDisplay();
}

//----------------------------------------------------------------------

void SwgCuiCalendar::performDeactivate()
{
	if (m_buttonPrevMonth)   m_buttonPrevMonth->RemoveCallback(this);
	if (m_buttonNextMonth)   m_buttonNextMonth->RemoveCallback(this);
	if (m_buttonToday)       m_buttonToday->RemoveCallback(this);
	if (m_buttonCreate)      m_buttonCreate->RemoveCallback(this);
	if (m_buttonSettings)    m_buttonSettings->RemoveCallback(this);
	if (m_buttonClose)       m_buttonClose->RemoveCallback(this);
	if (m_buttonViewDetails) m_buttonViewDetails->RemoveCallback(this);
	if (m_buttonDeleteEvent) m_buttonDeleteEvent->RemoveCallback(this);

	for (int i = 0; i < 42; ++i)
	{
		if (m_dayButtons[i])
			m_dayButtons[i]->RemoveCallback(this);
	}

	if (m_listEvents)
		m_listEvents->RemoveCallback(this);
}

//----------------------------------------------------------------------

void SwgCuiCalendar::OnButtonPressed(UIWidget *context)
{
	if (context == m_buttonPrevMonth)
	{
		navigateMonth(-1);
		requestEvents();
	}
	else if (context == m_buttonNextMonth)
	{
		navigateMonth(1);
		requestEvents();
	}
	else if (context == m_buttonToday)
	{
		goToToday();
		requestEvents();
	}
	else if (context == m_buttonCreate)
	{
		// Open event editor - no server notification needed until event is created
		CuiMediatorFactory::activateInWorkspace(CuiMediatorTypes::WS_CalendarEventEditor);
	}
	else if (context == m_buttonSettings)
	{
		// Open settings
		CuiMediatorFactory::activateInWorkspace(CuiMediatorTypes::WS_CalendarSettings);
	}
	else if (context == m_buttonClose)
	{
		deactivate();
	}
	else if (context == m_buttonViewDetails)
	{
		// Show event details from cached data
		// Display in a message box or dedicated panel
	}
	else if (context == m_buttonDeleteEvent)
	{
		// Find selected event and send delete message
		if (m_listEvents)
		{
			int selectedRow = m_listEvents->GetLastSelectedRow();
			if (selectedRow >= 0 && selectedRow < static_cast<int>(m_cachedEvents.size()))
			{
				CalendarEventData const & evt = m_cachedEvents[selectedRow];
				CalendarDeleteEventMessage msg(evt.eventId);
				GameNetwork::send(msg, true);
			}
		}
	}
	else
	{
		for (int i = 0; i < 42; ++i)
		{
			if (context == m_dayButtons[i])
			{
				int firstDay = getFirstDayOfMonth(m_currentYear, m_currentMonth);
				int daysInMonth = getDaysInMonth(m_currentYear, m_currentMonth);

				int cellNum = i + 1;
				int dayNum = cellNum - firstDay + 1;

				if (dayNum >= 1 && dayNum <= daysInMonth)
				{
					selectDay(dayNum);
				}
				break;
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiCalendar::OnGenericSelectionChanged(UIWidget *context)
{
	if (context == m_listEvents)
	{
	}
}

//----------------------------------------------------------------------

void SwgCuiCalendar::updateCalendarDisplay()
{
	if (m_textMonthYear && m_currentMonth >= 1 && m_currentMonth <= 12)
	{
		char buffer[64];
		snprintf(buffer, sizeof(buffer), "%s %d", s_monthNames[m_currentMonth - 1], m_currentYear);
		m_textMonthYear->SetText(Unicode::narrowToWide(buffer));
	}

	if (m_textSelectedDate && m_currentMonth >= 1 && m_currentMonth <= 12)
	{
		char buffer[64];
		snprintf(buffer, sizeof(buffer), "%s %d, %d", s_monthNames[m_currentMonth - 1], m_selectedDay, m_currentYear);
		m_textSelectedDate->SetText(Unicode::narrowToWide(buffer));
	}

	int daysInMonth = getDaysInMonth(m_currentYear, m_currentMonth);
	int firstDay = getFirstDayOfMonth(m_currentYear, m_currentMonth);

	time_t now = time(0);
	struct tm * timeinfo = localtime(&now);
	int todayYear = timeinfo ? timeinfo->tm_year + 1900 : 0;
	int todayMonth = timeinfo ? timeinfo->tm_mon + 1 : 0;
	int todayDay = timeinfo ? timeinfo->tm_mday : 0;

	int dayNum = 1;
	for (int cell = 0; cell < 42; ++cell)
	{
		UIButton * btn = m_dayButtons[cell];
		if (!btn)
			continue;

		int cellNum = cell + 1;

		if (cellNum < firstDay || dayNum > daysInMonth)
		{
			btn->SetVisible(false);
			btn->SetText(Unicode::emptyString);
		}
		else
		{
			btn->SetVisible(true);

			char dayText[16];
			snprintf(dayText, sizeof(dayText), "%d", dayNum);
			btn->SetText(Unicode::narrowToWide(dayText));

			bool isToday = (dayNum == todayDay && m_currentMonth == todayMonth && m_currentYear == todayYear);
			bool isSelected = (dayNum == m_selectedDay);

			UIColor bgColor;
			if (isSelected)
			{
				bgColor = UIColor(0, 255, 0);
			}
			else if (isToday)
			{
				bgColor = UIColor(255, 215, 0);
			}
			else
			{
				bgColor = UIColor(0, 51, 85);
			}

			btn->SetBackgroundTint(bgColor);

			dayNum++;
		}
	}

	refreshEventList();
}

//----------------------------------------------------------------------

void SwgCuiCalendar::selectDay(int day)
{
	int daysInMonth = getDaysInMonth(m_currentYear, m_currentMonth);
	if (day >= 1 && day <= daysInMonth)
	{
		m_selectedDay = day;
		updateCalendarDisplay();
	}
}

//----------------------------------------------------------------------

void SwgCuiCalendar::navigateMonth(int delta)
{
	m_currentMonth += delta;

	if (m_currentMonth > 12)
	{
		m_currentMonth = 1;
		m_currentYear++;
	}
	else if (m_currentMonth < 1)
	{
		m_currentMonth = 12;
		m_currentYear--;
	}

	int daysInMonth = getDaysInMonth(m_currentYear, m_currentMonth);
	if (m_selectedDay > daysInMonth)
	{
		m_selectedDay = daysInMonth;
	}

	updateCalendarDisplay();
}

//----------------------------------------------------------------------

void SwgCuiCalendar::goToToday()
{
	time_t now = time(0);
	struct tm * timeinfo = localtime(&now);
	if (timeinfo)
	{
		m_currentYear = timeinfo->tm_year + 1900;
		m_currentMonth = timeinfo->tm_mon + 1;
		m_selectedDay = timeinfo->tm_mday;
	}

	updateCalendarDisplay();
}

//----------------------------------------------------------------------

void SwgCuiCalendar::refreshEventList()
{
	if (m_dataEvents)
	{
		m_dataEvents->Clear();
	}

	if (m_listEvents)
	{
		m_listEvents->Clear();
	}

	// Populate list from cached events for the selected day
	for (std::vector<CalendarEventData>::const_iterator it = m_cachedEvents.begin(); it != m_cachedEvents.end(); ++it)
	{
		CalendarEventData const & evt = *it;

		// Filter to selected day
		if (evt.year == m_currentYear && evt.month == m_currentMonth && evt.day == m_selectedDay)
		{
			if (m_listEvents)
			{
				m_listEvents->AddRow(Unicode::narrowToWide(evt.title), evt.eventId);
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiCalendar::requestEvents()
{
	// Send request to server for events in current month
	CalendarGetEventsMessage msg(m_currentYear, m_currentMonth);
	GameNetwork::send(msg, true);
}

//----------------------------------------------------------------------

void SwgCuiCalendar::receiveMessage(MessageDispatch::Emitter const & /*source*/, MessageDispatch::MessageBase const & message)
{
	if (message.isType("CalendarEventsResponseMessage"))
	{
		Archive::ReadIterator ri = dynamic_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		CalendarEventsResponseMessage const msg(ri);
		onEventsResponse(msg.getEvents());
	}
	else if (message.isType("CalendarEventNotificationMessage"))
	{
		Archive::ReadIterator ri = dynamic_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		CalendarEventNotificationMessage const msg(ri);
		onEventNotification(msg.getNotificationType(), msg.getEventData());
	}
	else if (message.isType("CalendarDeleteEventResponseMessage"))
	{
		Archive::ReadIterator ri = dynamic_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		CalendarDeleteEventResponseMessage const msg(ri);
		if (msg.getSuccess())
		{
			// Refresh events
			requestEvents();
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiCalendar::onEventsResponse(std::vector<CalendarEventData> const & events)
{
	m_cachedEvents = events;
	refreshEventList();
	updateCalendarDisplay();
}

//----------------------------------------------------------------------

void SwgCuiCalendar::onEventNotification(int notificationType, CalendarEventData const & eventData)
{
	// Handle notification - refresh the event list
	switch (notificationType)
	{
		case 0: // Created
		case 1: // Updated
		case 2: // Deleted
			// Refresh from server
			requestEvents();
			break;

		case 3: // Started
		case 4: // Ended
			// Show a notification to the user
			break;
	}
}

//======================================================================
//======================================================================
