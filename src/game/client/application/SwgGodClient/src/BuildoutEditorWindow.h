// ======================================================================
//
// BuildoutEditorWindow.h
// copyright 2024 Sony Online Entertainment
//
// Renders the planet map with an overlay grid of buildout areas.
// Allows selection of areas to save en masse. 1024x768 window with
// a Save button that activates when at least one area is selected.
//
// ======================================================================

#ifndef INCLUDED_BuildoutEditorWindow_H
#define INCLUDED_BuildoutEditorWindow_H

// ======================================================================

#include <qdialog.h>
#include <string>
#include <vector>

class QPushButton;
class QLabel;
class QComboBox;

// ======================================================================

class BuildoutAreaWidget : public QWidget
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762

public:
	explicit BuildoutAreaWidget(QWidget * parent = 0, const char * name = 0);
	virtual ~BuildoutAreaWidget();

	struct BuildoutArea
	{
		std::string name;
		float       x1;
		float       z1;
		float       x2;
		float       z2;
		bool        selected;
	};

	void setPlanetName(const std::string & planet);
	void setBuildoutAreas(const std::vector<BuildoutArea> & areas);
	const std::vector<BuildoutArea> & getBuildoutAreas() const;
	int  getSelectedCount() const;
	std::vector<std::string> getSelectedAreaNames() const;

signals:
	void selectionChanged(int selectedCount);

protected:
	virtual void paintEvent(QPaintEvent * event);
	virtual void mousePressEvent(QMouseEvent * event);

private:
	BuildoutAreaWidget(const BuildoutAreaWidget &);
	BuildoutAreaWidget & operator=(const BuildoutAreaWidget &);

	int findAreaAtPoint(int px, int py) const;
	void worldToScreen(float wx, float wz, int & sx, int & sy) const;

	std::string m_planetName;
	std::vector<BuildoutArea> m_areas;

	float m_worldMinX;
	float m_worldMinZ;
	float m_worldMaxX;
	float m_worldMaxZ;
};

// ======================================================================

class BuildoutEditorWindow : public QDialog
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762

public:

	explicit BuildoutEditorWindow(QWidget * parent = 0, const char * name = 0);
	virtual ~BuildoutEditorWindow();

signals:
	void statusMessage(const char * msg);

public slots:
	void onSave();
	void onPlanetChanged(const QString & planet);
	void onSelectionChanged(int count);

private:
	BuildoutEditorWindow(const BuildoutEditorWindow &);
	BuildoutEditorWindow & operator=(const BuildoutEditorWindow &);

	void loadPlanetBuildouts(const std::string & planet);

	BuildoutAreaWidget * m_areaWidget;
	QPushButton *        m_saveButton;
	QLabel *             m_statusLabel;
	QComboBox *          m_planetCombo;
	std::string          m_currentPlanet;
};

// ======================================================================

#endif
