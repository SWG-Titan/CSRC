//======================================================================
//
// SwgCuiCalendarEventDetails.h
// copyright (c) 2026 Titan Project
//
// In-Game Calendar System - Event Details Popup
//
//======================================================================

#ifndef INCLUDED_SwgCuiCalendarEventDetails_H
#define INCLUDED_SwgCuiCalendarEventDetails_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "sharedNetworkMessages/CalendarMessages.h"
#include "UIEventCallback.h"

class UIPage;
class UIButton;
class UIText;
class UIImage;

// ======================================================================

class SwgCuiCalendarEventDetails :
public CuiMediator,
public UIEventCallback
{
public:

	explicit                       SwgCuiCalendarEventDetails    (UIPage & page);

	virtual void                   OnButtonPressed               (UIWidget *context);

	virtual void                   performActivate               ();
	virtual void                   performDeactivate             ();

	void                           setEventData                  (CalendarEventData const & eventData);

	static SwgCuiCalendarEventDetails * getInstance              ();
	static void                    showDetails                   (CalendarEventData const & eventData);

private:
	virtual                       ~SwgCuiCalendarEventDetails    ();
	                               SwgCuiCalendarEventDetails    ();
	                               SwgCuiCalendarEventDetails    (const SwgCuiCalendarEventDetails & rhs);
	SwgCuiCalendarEventDetails &   operator=                     (const SwgCuiCalendarEventDetails & rhs);

	void                           updateDisplay                 ();
	void                           deleteEvent                   ();

private:
	UIText *                       m_textTitle;
	UIText *                       m_textType;
	UIText *                       m_textDate;
	UIText *                       m_textTime;
	UIText *                       m_textDescription;
	UIImage *                      m_imageEvent;
	UIButton *                     m_buttonClose;
	UIButton *                     m_buttonEdit;
	UIButton *                     m_buttonDelete;

	CalendarEventData              m_currentEvent;

	static SwgCuiCalendarEventDetails * ms_instance;
};

//======================================================================

#endif


