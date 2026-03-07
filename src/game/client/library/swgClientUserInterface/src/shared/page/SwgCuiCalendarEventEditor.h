//======================================================================
//
// SwgCuiCalendarEventEditor.h
// copyright (c) 2026 Titan Project
//
// In-Game Calendar System - Event Editor Dialog
//
//======================================================================

#ifndef INCLUDED_SwgCuiCalendarEventEditor_H
#define INCLUDED_SwgCuiCalendarEventEditor_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class UIPage;
class UIButton;
class UIText;
class UITextbox;
class UIComboBox;
class UICheckbox;

// ======================================================================

class SwgCuiCalendarEventEditor :
public CuiMediator,
public UIEventCallback
{
public:

	explicit                      SwgCuiCalendarEventEditor (UIPage & page);

	virtual void                  OnButtonPressed           (UIWidget *context);
	virtual void                  OnCheckboxSet             (UIWidget *context);
	virtual void                  OnCheckboxUnset           (UIWidget *context);
	virtual void                  OnGenericSelectionChanged (UIWidget *context);

	virtual void                  performActivate           ();
	virtual void                  performDeactivate         ();

	void                          setDefaultDate            (int year, int month, int day);

private:
	virtual                      ~SwgCuiCalendarEventEditor ();
	                              SwgCuiCalendarEventEditor ();
	                              SwgCuiCalendarEventEditor (const SwgCuiCalendarEventEditor & rhs);
	SwgCuiCalendarEventEditor &   operator=                 (const SwgCuiCalendarEventEditor & rhs);

	void                          updateRecurrenceVisibility();
	void                          updateServerEventVisibility();
	void                          createEvent();

private:
	UITextbox *                   m_textTitle;
	UITextbox *                   m_textDescription;
	UIComboBox *                  m_comboEventType;
	UITextbox *                   m_textDate;
	UITextbox *                   m_textTime;
	UIComboBox *                  m_comboDuration;
	UIPage *                      m_pageServerEvent;
	UIComboBox *                  m_comboServerEvent;
	UICheckbox *                  m_checkBroadcast;
	UICheckbox *                  m_checkRecurring;
	UIPage *                      m_pageRecurrence;
	UIComboBox *                  m_comboRecurrence;
	UIButton *                    m_buttonCreate;
	UIButton *                    m_buttonCancel;
};

//======================================================================

#endif

