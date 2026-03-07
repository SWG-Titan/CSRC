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
#include "UIEventCallback.h"

#include <vector>

class UIPage;
class UIButton;
class UIText;
class UIList;
class UIDataSource;
struct CalendarEventData;

namespace MessageDispatch
{
	class Callback;
}

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

	// Network message handlers
	void                     onEventsResponse(std::vector<CalendarEventData> const & events);
	void                     onEventNotification(int notificationType, CalendarEventData const & eventData);

	// Request events from server
	void                     requestEvents();

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

	int                      m_currentYear;
	int                      m_currentMonth;
	int                      m_selectedDay;

	// Cached events from server
	std::vector<CalendarEventData> m_cachedEvents;

	// Message callback
	MessageDispatch::Callback * m_callback;
};

//======================================================================

#endif

