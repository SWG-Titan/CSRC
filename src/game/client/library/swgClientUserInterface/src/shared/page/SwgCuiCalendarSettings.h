//======================================================================
//
// SwgCuiCalendarSettings.h
// copyright (c) 2026 Titan Project
//
// In-Game Calendar System - Settings Dialog (Staff Only)
//
//======================================================================

#ifndef INCLUDED_SwgCuiCalendarSettings_H
#define INCLUDED_SwgCuiCalendarSettings_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class UIPage;
class UIButton;
class UITextbox;
class UIImage;

// ======================================================================

class SwgCuiCalendarSettings :
public CuiMediator,
public UIEventCallback
{
public:

	explicit                    SwgCuiCalendarSettings (UIPage & page);

	virtual void                OnButtonPressed        (UIWidget *context);

	virtual void                performActivate        ();
	virtual void                performDeactivate      ();

	void                        loadSettings           ();
	void                        applySettings          ();
	void                        resetSettings          ();
	void                        updatePreview          ();

private:
	virtual                    ~SwgCuiCalendarSettings ();
	                            SwgCuiCalendarSettings ();
	                            SwgCuiCalendarSettings (const SwgCuiCalendarSettings & rhs);
	SwgCuiCalendarSettings &    operator=              (const SwgCuiCalendarSettings & rhs);

private:
	UITextbox *                 m_textTexture;
	UITextbox *                 m_textRectX;
	UITextbox *                 m_textRectY;
	UITextbox *                 m_textRectW;
	UITextbox *                 m_textRectH;
	UIImage *                   m_imagePreview;
	UIButton *                  m_buttonPreview;
	UIButton *                  m_buttonApply;
	UIButton *                  m_buttonReset;
	UIButton *                  m_buttonClose;
};

//======================================================================

#endif

