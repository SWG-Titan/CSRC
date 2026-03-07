//======================================================================
//
// CuiColorPicker.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiColorPicker_H
#define INCLUDED_CuiColorPicker_H

//======================================================================

#include "UIEventCallback.h"
#include "clientUserInterface/CuiMediator.h"
#include "sharedMath/PackedArgb.h"

class CachedNetworkId;
class CustomizationManagerPaletteColumns;
class NetworkId;
class Object;
class PaletteArgb;
class TangibleObject;
class UIButton;
class UIDataSource;
class UILowerString;
class UIPage;
class UISliderbar;
class UIText;
class UITextbox;
class UIVolumePage;
class UIWidget;

template <typename T> class Watcher;

// ======================================================================

/**
* CuiColorPicker
*/

class CuiColorPicker :
public CuiMediator,
public UIEventCallback
{
public:

	typedef Watcher<TangibleObject>       ObjectWatcher;
	typedef stdvector<ObjectWatcher>::fwd ObjectWatcherVector;

	struct Properties
	{
		static const UILowerString AutoSizePaletteCells;
	};

	struct DataProperties
	{
		static const UILowerString  TargetNetworkId;
		static const UILowerString  TargetVariable;
		static const UILowerString  TargetValue;
		static const UILowerString  TargetRangeMin;
		static const UILowerString  TargetRangeMax;
		static const UILowerString  SelectedIndex;
	};

	enum PathFlags
	{
		PF_shared   = 0x0001,
		PF_private  = 0x0002,
		PF_any      = 0x0003
	};

	explicit                      CuiColorPicker (UIPage & page);

	virtual void                  performActivate   ();
	virtual void                  performDeactivate ();

	virtual void                  OnButtonPressed              (UIWidget * context);
	virtual void                  OnVolumePageSelectionChanged (UIWidget * context);
	virtual void                  OnTextboxChanged             (UIWidget * context);
	virtual bool                  OnMessage                    (UIWidget * context, const UIMessage & msg);

	void                          setTarget                    (const NetworkId & id, const std::string & var, int rangeMin, int rangeMax);
	void                          setPalette(PaletteArgb const * palette);
	PaletteArgb const *           getPalette() const;
	void                          setTarget                    (Object * obj, const std::string & var, int rangeMin, int rangeMax);
	void                          setIndex(int index);

	int                           getValue                     () const;
	const std::string &           getTargetVariable            () const;

	void                          setLinkedObjects             (const ObjectWatcherVector & v, bool doUpdate);
	const ObjectWatcherVector &   getLinkedObjects             () const;

	void                          updateCellSizes              ();

	void                          setText                      (const Unicode::String & str);

	void                          setForceColumns              (int cols);
	void                          setAutoForceColumns          (bool b);
	void                          setMaximumPaletteIndex       (int index);

//	typedef stdmap<std::string, int>::fwd StringIntMap;
//	static void                   setupPaletteColumnData       (const UIDataSource & ds);
	static void                   setupPaletteColumnData       (stdmap<std::string, CustomizationManagerPaletteColumns>::fwd const & data);


	bool                          checkAndResetChanged         ();
	bool                          checkAndResetUSerChanged     ();

	void                          update                       (float deltaTimeSecs);
	void                          reset                        ();

	void                          handleMediatorPropertiesChanged ();

	// Color wheel / HTML color support
	void                          setColorWheelMode            (bool enabled);
	bool                          isColorWheelMode             () const;
	void                          setColorFromHtml             (const char * htmlColor);
	void                          getColorAsHtml               (char * buffer, int bufferSize) const;
	void                          setDirectColor               (const PackedArgb & color);
	const PackedArgb &            getDirectColor               () const;
	bool                          hasDirectColor               () const;

private:
	                             ~CuiColorPicker ();
	                              CuiColorPicker ();
	                              CuiColorPicker (const CuiColorPicker & rhs);
	CuiColorPicker &              operator=      (const CuiColorPicker & rhs);

private:

	void                     updateValue     (int index);
	void                     updateValue     (TangibleObject & obj, int index, PathFlags flags = PF_any);
	void                     updateValueDirectColor (TangibleObject & obj, const PackedArgb & color, PathFlags flags = PF_any);
	void                     revert          ();
	void                     storeProperties ();
	void                     onColorWheelChanged (const PackedArgb & color);
	void                     updateHtmlTextbox ();

	// Color wheel helpers
	void                     updateFromHSV();
	void                     updateFromRGB();
	void                     updateWheelCursor();
	void                     updateRgbTextboxes();
	void                     handleWheelInput(int x, int y);
	static void              rgbToHsv(uint8 r, uint8 g, uint8 b, float & h, float & s, float & v);
	static void              hsvToRgb(float h, float s, float v, uint8 & r, uint8 & g, uint8 & b);

	UIVolumePage *           m_volumePage;
	UIButton *               m_buttonCancel;
	UIButton *               m_buttonRevert;
	UIButton *               m_buttonClose;
	UIPage *                 m_pageSample;

	int                      m_originalSelection;
	int                      m_rangeMin;
	int                      m_rangeMax;
	PaletteArgb const *      m_palette;
	ObjectWatcher *          m_targetObject;
	std::string              m_targetVariable;

	UIWidget *               m_sampleElement;

	ObjectWatcherVector *    m_linkedObjects;
	bool                     m_autoSizePaletteCells;

	UIText *                 m_text;

	int                      m_forceColumns;

	bool                     m_autoForceColumns;

	bool                     m_changed;
	bool                     m_userChanged;
	UISize                   m_lastSize;

	enum PaletteSource
	{
		PS_target,
		PS_palette
	};

	PaletteSource            m_paletteSource;

	// Color wheel / HTML color support
	bool                     m_colorWheelMode;
	UIButton *               m_buttonToggleMode;    // Toggle between palette grid and color wheel
	UIPage *                 m_pageColorWheel;      // Color wheel container page
	UIPage *                 m_pagePaletteGrid;     // Palette grid container page
	UITextbox *              m_textboxHtml;         // HTML hex color input
	UIPage *                 m_pageMatchedColor;    // Shows matched palette color when using direct color
	PackedArgb               m_directColor;         // Current direct color (when using wheel/HTML)
	bool                     m_useDirectColor;      // True if using direct color vs palette index
	PackedArgb               m_originalDirectColor; // Original direct color for revert

	// Color wheel interactive elements
	UIPage *                 m_pageWheel;           // Wheel page (contains image + cursor)
	UIWidget *               m_cursorWheel;         // Cursor on the wheel
	UITextbox *              m_textboxR;            // Red value textbox
	UITextbox *              m_textboxG;            // Green value textbox
	UITextbox *              m_textboxB;            // Blue value textbox

	// HSV state for color wheel
	float                    m_hue;                 // 0.0 - 360.0
	float                    m_saturation;          // 0.0 - 1.0
	float                    m_colorValue;          // 0.0 - 1.0 (renamed from value to avoid keyword)
	bool                     m_draggingWheel;
};

//----------------------------------------------------------------------

inline const CuiColorPicker::ObjectWatcherVector & CuiColorPicker::getLinkedObjects             () const
{
	return *NON_NULL (m_linkedObjects);
}

//----------------------------------------------------------------------

inline const std::string & CuiColorPicker::getTargetVariable            () const
{
	return m_targetVariable;
}

//----------------------------------------------------------------------

inline bool CuiColorPicker::checkAndResetChanged ()
{
	const bool val = m_changed;
	m_changed = false;
	return val;
}

//----------------------------------------------------------------------

inline bool CuiColorPicker::checkAndResetUSerChanged ()
{
	bool val = m_userChanged;
	m_userChanged = false;
	return val;
}

//======================================================================

#endif

