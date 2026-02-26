// ======================================================================
//
// BuildoutEditorWindow.cpp
// copyright 2024 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "BuildoutEditorWindow.h"
#include "BuildoutEditorWindow.moc"

#include "MainFrame.h"

#include "FileControlServer.h"

#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qpushbutton.h>

#include <cstdio>
#include <cstring>
#include <fstream>
#include <sys/stat.h>

// ======================================================================
// BuildoutAreaWidget
// ======================================================================

BuildoutAreaWidget::BuildoutAreaWidget(QWidget * parent, const char * name)
: QWidget(parent, name),
  m_planetName(),
  m_areas(),
  m_worldMinX(-8192.0f),
  m_worldMinZ(-8192.0f),
  m_worldMaxX(8192.0f),
  m_worldMaxZ(8192.0f)
{
	setMinimumSize(800, 600);
	setBackgroundColor(QColor(20, 30, 20));
}

// ----------------------------------------------------------------------

BuildoutAreaWidget::~BuildoutAreaWidget()
{
}

// ----------------------------------------------------------------------

void BuildoutAreaWidget::setPlanetName(const std::string & planet)
{
	m_planetName = planet;
	update();
}

// ----------------------------------------------------------------------

void BuildoutAreaWidget::setBuildoutAreas(const std::vector<BuildoutArea> & areas)
{
	m_areas = areas;
	update();
}

// ----------------------------------------------------------------------

const std::vector<BuildoutAreaWidget::BuildoutArea> & BuildoutAreaWidget::getBuildoutAreas() const
{
	return m_areas;
}

// ----------------------------------------------------------------------

int BuildoutAreaWidget::getSelectedCount() const
{
	int count = 0;
	for (size_t i = 0; i < m_areas.size(); ++i)
	{
		if (m_areas[i].selected)
			++count;
	}
	return count;
}

// ----------------------------------------------------------------------

std::vector<std::string> BuildoutAreaWidget::getSelectedAreaNames() const
{
	std::vector<std::string> names;
	for (size_t i = 0; i < m_areas.size(); ++i)
	{
		if (m_areas[i].selected)
			names.push_back(m_areas[i].name);
	}
	return names;
}

// ----------------------------------------------------------------------

void BuildoutAreaWidget::paintEvent(QPaintEvent * event)
{
	UNREF(event);

	QPainter painter(this);

	painter.fillRect(rect(), QColor(20, 30, 20));

	for (size_t i = 0; i < m_areas.size(); ++i)
	{
		const BuildoutArea & area = m_areas[i];

		int sx1, sy1, sx2, sy2;
		worldToScreen(area.x1, area.z1, sx1, sy1);
		worldToScreen(area.x2, area.z2, sx2, sy2);

		if (sx1 > sx2) { int t = sx1; sx1 = sx2; sx2 = t; }
		if (sy1 > sy2) { int t = sy1; sy1 = sy2; sy2 = t; }

		int w = sx2 - sx1;
		int h = sy2 - sy1;

		if (area.selected)
		{
			painter.fillRect(sx1, sy1, w, h, QBrush(QColor(60, 120, 60)));
			painter.setPen(QColor(100, 255, 100));
		}
		else
		{
			painter.fillRect(sx1, sy1, w, h, QBrush(QColor(30, 50, 30)));
			painter.setPen(QColor(60, 90, 60));
		}

		painter.drawRect(sx1, sy1, w, h);

		painter.setPen(area.selected ? QColor(200, 255, 200) : QColor(100, 140, 100));

		std::string label = area.name;
		std::string::size_type lastUnderscore = label.rfind('_');
		std::string::size_type secondLast = (lastUnderscore != std::string::npos && lastUnderscore > 0)
			? label.rfind('_', lastUnderscore - 1) : std::string::npos;
		if (secondLast != std::string::npos)
			label = label.substr(secondLast + 1);

		int textX = sx1 + (w - painter.fontMetrics().width(label.c_str())) / 2;
		int textY = sy1 + (h + painter.fontMetrics().ascent()) / 2;
		painter.drawText(textX, textY, label.c_str());
	}

	painter.setPen(QColor(160, 200, 160));
	painter.drawText(4, painter.fontMetrics().ascent() + 2,
		m_planetName.empty() ? "No planet selected" : m_planetName.c_str());
}

// ----------------------------------------------------------------------

void BuildoutAreaWidget::mousePressEvent(QMouseEvent * event)
{
	int idx = findAreaAtPoint(event->x(), event->y());
	if (idx >= 0)
	{
		m_areas[static_cast<size_t>(idx)].selected = !m_areas[static_cast<size_t>(idx)].selected;
		update();
		emit selectionChanged(getSelectedCount());
	}
}

// ----------------------------------------------------------------------

int BuildoutAreaWidget::findAreaAtPoint(int px, int py) const
{
	for (size_t i = 0; i < m_areas.size(); ++i)
	{
		const BuildoutArea & area = m_areas[i];
		int sx1, sy1, sx2, sy2;
		worldToScreen(area.x1, area.z1, sx1, sy1);
		worldToScreen(area.x2, area.z2, sx2, sy2);

		if (sx1 > sx2) { int t = sx1; sx1 = sx2; sx2 = t; }
		if (sy1 > sy2) { int t = sy1; sy1 = sy2; sy2 = t; }

		if (px >= sx1 && px <= sx2 && py >= sy1 && py <= sy2)
			return static_cast<int>(i);
	}
	return -1;
}

// ----------------------------------------------------------------------

void BuildoutAreaWidget::worldToScreen(float wx, float wz, int & sx, int & sy) const
{
	float rangeX = m_worldMaxX - m_worldMinX;
	float rangeZ = m_worldMaxZ - m_worldMinZ;

	if (rangeX <= 0.0f) rangeX = 1.0f;
	if (rangeZ <= 0.0f) rangeZ = 1.0f;

	sx = static_cast<int>(((wx - m_worldMinX) / rangeX) * static_cast<float>(width()));
	sy = static_cast<int>(((wz - m_worldMinZ) / rangeZ) * static_cast<float>(height()));
}

// ======================================================================
// BuildoutEditorWindow
// ======================================================================

BuildoutEditorWindow::BuildoutEditorWindow(QWidget * parent, const char * name)
: QDialog(parent, name, false),
  m_areaWidget(0),
  m_saveButton(0),
  m_statusLabel(0),
  m_planetCombo(0),
  m_currentPlanet()
{
	setCaption("Buildout Editor");
	resize(1024, 768);

	QVBoxLayout * mainLayout = new QVBoxLayout(this, 0, 0);

	QHBoxLayout * topLayout = new QHBoxLayout(0, 4, 4);
	QLabel * planetLabel = new QLabel("Planet:", this);
	m_planetCombo = new QComboBox(this);
	m_planetCombo->insertItem("tatooine");
	m_planetCombo->insertItem("naboo");
	m_planetCombo->insertItem("corellia");
	m_planetCombo->insertItem("dantooine");
	m_planetCombo->insertItem("dathomir");
	m_planetCombo->insertItem("endor");
	m_planetCombo->insertItem("lok");
	m_planetCombo->insertItem("rori");
	m_planetCombo->insertItem("talus");
	m_planetCombo->insertItem("yavin4");
	m_planetCombo->insertItem("kashyyyk_main");
	m_planetCombo->insertItem("mustafar");
	topLayout->addWidget(planetLabel);
	topLayout->addWidget(m_planetCombo);
	topLayout->addStretch(1);
	mainLayout->addLayout(topLayout);

	m_areaWidget = new BuildoutAreaWidget(this, "buildoutAreaWidget");
	mainLayout->addWidget(m_areaWidget, 1);

	m_statusLabel = new QLabel("Select a planet and click buildout areas to select them.", this);
	mainLayout->addWidget(m_statusLabel);

	QHBoxLayout * buttonLayout = new QHBoxLayout(0, 4, 6);
	buttonLayout->addStretch(1);
	m_saveButton = new QPushButton("Save Selected Buildouts", this);
	m_saveButton->setEnabled(false);
	buttonLayout->addWidget(m_saveButton);
	mainLayout->addLayout(buttonLayout);

	IGNORE_RETURN(connect(m_saveButton,   SIGNAL(clicked()), this, SLOT(onSave())));
	IGNORE_RETURN(connect(m_planetCombo,  SIGNAL(activated(const QString &)), this, SLOT(onPlanetChanged(const QString &))));
	IGNORE_RETURN(connect(m_areaWidget,   SIGNAL(selectionChanged(int)), this, SLOT(onSelectionChanged(int))));
}

// ----------------------------------------------------------------------

BuildoutEditorWindow::~BuildoutEditorWindow()
{
}

// ----------------------------------------------------------------------

void BuildoutEditorWindow::onPlanetChanged(const QString & planet)
{
	m_currentPlanet = planet.latin1();
	loadPlanetBuildouts(m_currentPlanet);
}

// ----------------------------------------------------------------------

void BuildoutEditorWindow::onSelectionChanged(int count)
{
	m_saveButton->setEnabled(count > 0);
	m_statusLabel->setText(QString("%1 buildout area(s) selected.").arg(count));
}

// ----------------------------------------------------------------------

void BuildoutEditorWindow::onSave()
{
	std::vector<std::string> selected = m_areaWidget->getSelectedAreaNames();
	if (selected.empty())
	{
		QMessageBox::warning(this, "Buildout Editor", "No buildout areas selected.");
		return;
	}

	QString msg = QString("Save %1 buildout area(s) for %2?\n\n").arg(static_cast<int>(selected.size())).arg(m_currentPlanet.c_str());
	for (size_t i = 0; i < selected.size(); ++i)
		msg += QString("  - %1\n").arg(selected[i].c_str());

	int result = QMessageBox::question(this, "Save Buildouts", msg, QMessageBox::Yes, QMessageBox::No);
	if (result != QMessageBox::Yes)
		return;

	m_statusLabel->setText("Saving buildout areas...");

	int successCount = 0;
	for (size_t i = 0; i < selected.size(); ++i)
	{
		std::string buildoutPath = "data/sku.0/sys.server/compiled/game/buildout/" + m_currentPlanet + "/" + selected[i] + ".iff";

		bool sent = FileControlServer::requestSendAsset(buildoutPath);
		if (sent)
		{
			++successCount;
			MainFrame::getInstance().textToConsole(
				("BuildoutEditor: Sent " + selected[i] + " on " + m_currentPlanet).c_str());
		}
		else
		{
			MainFrame::getInstance().textToConsole(
				("BuildoutEditor: FAILED to send " + selected[i] + " on " + m_currentPlanet).c_str());
		}
	}

	if (successCount > 0)
	{
		FileControlServer::requestReloadAsset(
			"data/sku.0/sys.server/compiled/game/buildout/" + m_currentPlanet + "/");
	}

	m_statusLabel->setText(QString("Saved %1/%2 buildout area(s). Compile on server and distribute.")
		.arg(successCount).arg(static_cast<int>(selected.size())));
	emit statusMessage("Buildout areas saved and distributed.");
}

// ----------------------------------------------------------------------

void BuildoutEditorWindow::loadPlanetBuildouts(const std::string & planet)
{
	m_areaWidget->setPlanetName(planet);

	std::vector<BuildoutAreaWidget::BuildoutArea> areas;

	std::string buildoutDir = "data/sku.0/sys.server/compiled/game/buildout/" + planet + "/";
	std::vector<std::string> files;
	std::vector<unsigned long> sizes;
	std::vector<unsigned long> crcs;

	bool gotListing = FileControlServer::requestDirectoryListing(buildoutDir, files, sizes, crcs);

	const float worldMin  = -8192.0f;
	const float worldSize = 16384.0f;
	const int   gridSize  = 8;
	const float cellSize  = worldSize / static_cast<float>(gridSize);

	if (gotListing && !files.empty())
	{
		for (size_t i = 0; i < files.size(); ++i)
		{
			std::string name = files[i];
			std::string::size_type lastSlash = name.find_last_of("/\\");
			if (lastSlash != std::string::npos)
				name = name.substr(lastSlash + 1);
			std::string::size_type dotPos = name.rfind('.');
			if (dotPos != std::string::npos)
				name = name.substr(0, dotPos);

			int row = 0, col = 0;
			if (sscanf(name.c_str(), (planet + "_%d_%d").c_str(), &row, &col) == 2
				&& row >= 1 && row <= gridSize && col >= 1 && col <= gridSize)
			{
				continue;
			}

			BuildoutAreaWidget::BuildoutArea area;
			area.selected = false;
			area.name = name;
			area.x1 = worldMin;
			area.z1 = worldMin;
			area.x2 = worldMin + cellSize * 0.5f;
			area.z2 = worldMin + cellSize * 0.5f;
			areas.push_back(area);
		}
	}

	for (int row = 1; row <= gridSize; ++row)
	{
		for (int col = 1; col <= gridSize; ++col)
		{
			char cellName[128];
			snprintf(cellName, sizeof(cellName), "%s_%d_%d", planet.c_str(), row, col);

			BuildoutAreaWidget::BuildoutArea area;
			area.selected = false;
			area.name     = cellName;
			area.x1       = worldMin + static_cast<float>(col - 1) * cellSize;
			area.z1       = worldMin + static_cast<float>(row - 1) * cellSize;
			area.x2       = area.x1 + cellSize;
			area.z2       = area.z1 + cellSize;
			areas.push_back(area);
		}
	}

	m_areaWidget->setBuildoutAreas(areas);
	m_statusLabel->setText(QString("Loaded %1 buildout areas for %2.").arg(static_cast<int>(areas.size())).arg(planet.c_str()));
}

// ======================================================================
