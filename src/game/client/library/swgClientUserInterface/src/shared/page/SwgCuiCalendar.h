//======================================================================
//
// SwgCuiCalendar.h
// copyright (c) 2026 Titan Project
//
// In-Game Calendar System - Main Calendar Window
//
//======================================================================

#ifndef INCLUDED_SwgCuiCalendar_H
#define INCLUDED_SwgCuiCalendar_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedNetworkMessages/CalendarMessages.h"
#include "UIEventCallback.h"

#include <vector>
#include <string>

class UIPage;
class UIButton;
class UIText;
class UIList;
class UIDataSource;
class UIImage;

// ======================================================================

class SwgCuiCalendar :
public CuiMediator,
public UIEventCallback,
public MessageDispatch::Receiver
{
public:

	explicit                 SwgCuiCalendar    (UIPage & page);

	virtual void             OnButtonPressed   (UIWidget *context);
	virtual void             OnGenericSelectionChanged(UIWidget *context);

	virtual void             performActivate   ();
	virtual void             performDeactivate ();

	// MessageDispatch::Receiver
	virtual void             receiveMessage    (MessageDispatch::Emitter const & source, MessageDispatch::MessageBase const & message);

	void                     updateCalendarDisplay();
	void                     selectDay(int day);
	void                     navigateMonth(int delta);
	void                     goToToday();
	void                     refreshEventList();
	void                     showEventDetails();

	// Network message handlers
	void                     onEventsResponse(std::vector<CalendarEventData> const & events);
	void                     onEventNotification(int notificationType, CalendarEventData const & eventData);

	// Request events from server
	void                     requestEvents();

	// Static accessors for passing selected date to event editor
	static int               getSelectedYear();
	static int               getSelectedMonth();
	static int               getSelectedDay();

private:
	virtual                 ~SwgCuiCalendar ();
	                         SwgCuiCalendar ();
	                         SwgCuiCalendar (const SwgCuiCalendar & rhs);
	SwgCuiCalendar &         operator=      (const SwgCuiCalendar & rhs);

private:
	UIButton *               m_buttonPrevMonth;
	UIButton *               m_buttonNextMonth;
	UIButton *               m_buttonToday;
	UIButton *               m_buttonCreate;
	UIButton *               m_buttonSettings;
	UIButton *               m_buttonClose;
	UIText *                 m_textMonthYear;

	UIButton *               m_dayButtons[42];

	UIText *                 m_textSelectedDate;
	UIList *                 m_listEvents;
	UIDataSource *           m_dataEvents;
	UIButton *               m_buttonViewDetails;
	UIButton *               m_buttonDeleteEvent;

	// Detail panel
	UIPage *                 m_pageDetail;
	UIText *                 m_textDetailTitle;
	UIText *                 m_textDetailDesc;
	UIText *                 m_textDetailTime;
	UIText *                 m_textDetailType;
	UIImage *                m_imageDetailImage;
	UIButton *               m_buttonDetailClose;

	int                      m_currentYear;
	int                      m_currentMonth;
	int                      m_selectedDay;

	// Cached events from server
	std::vector<CalendarEventData> m_cachedEvents;
	// Filtered events for selected day
	std::vector<CalendarEventData> m_selectedDayEvents;

	static int               ms_selectedYear;
	static int               ms_selectedMonth;
	static int               ms_selectedDay;
};

//======================================================================

#endif

