// ======================================================================
//
// TemplateEditorWindow.cpp
// copyright 2024 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "TemplateEditorWindow.h"
#include "TemplateEditorWindow.moc"

#include "MainFrame.h"

#include <qcombobox.h>
#include <qfiledialog.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qsplitter.h>
#include <qtextedit.h>

#include <cstdio>
#include <fstream>

// ======================================================================

TemplateEditorWindow::TemplateEditorWindow(QWidget * parent, const char * name)
: QDialog(parent, name, false),
  m_serverEditor(0),
  m_sharedEditor(0),
  m_stringEditor(0),
  m_serverPathEdit(0),
  m_sharedPathEdit(0),
  m_stringPathEdit(0),
  m_saveButton(0),
  m_compileButton(0),
  m_newButton(0),
  m_browseServerButton(0),
  m_browseSharedButton(0),
  m_browseStringButton(0),
  m_templateTypeCombo(0),
  m_statusLabel(0),
  m_currentServerPath(),
  m_currentSharedPath(),
  m_currentStringPath(),
  m_hasLoadedTemplate(false)
{
	setCaption("Template Editor");
	resize(1000, 800);

	QVBoxLayout * mainLayout = new QVBoxLayout(this, 6, 6);

	// New template bar
	QHBoxLayout * newLayout = new QHBoxLayout(0, 0, 4);
	QLabel * typeLabel = new QLabel("Type:", this);
	m_templateTypeCombo = new QComboBox(this);
	m_templateTypeCombo->insertItem("tangible");
	m_templateTypeCombo->insertItem("creature");
	m_templateTypeCombo->insertItem("weapon");
	m_templateTypeCombo->insertItem("armor");
	m_templateTypeCombo->insertItem("building");
	m_templateTypeCombo->insertItem("installation");
	m_templateTypeCombo->insertItem("vehicle");
	m_templateTypeCombo->insertItem("static");
	m_templateTypeCombo->insertItem("deed");
	m_templateTypeCombo->insertItem("resource_container");
	m_templateTypeCombo->insertItem("draft_schematic");
	m_templateTypeCombo->insertItem("mission");
	m_templateTypeCombo->insertItem("waypoint");
	m_templateTypeCombo->insertItem("cell");
	m_templateTypeCombo->insertItem("intangible");
	m_newButton = new QPushButton("New Template", this);
	newLayout->addWidget(typeLabel);
	newLayout->addWidget(m_templateTypeCombo);
	newLayout->addWidget(m_newButton);
	newLayout->addStretch(1);
	mainLayout->addLayout(newLayout);

	// Top splitter: server (left) and shared (right) editors
	QSplitter * topSplitter = new QSplitter(Qt::Horizontal, this);

	QWidget * serverPane = new QWidget(topSplitter);
	QVBoxLayout * serverLayout = new QVBoxLayout(serverPane, 2, 2);
	QLabel * serverLabel = new QLabel("Server Template (.tpf)", serverPane);
	serverLayout->addWidget(serverLabel);

	QHBoxLayout * serverPathLayout = new QHBoxLayout(0, 0, 2);
	m_serverPathEdit = new QLineEdit(serverPane);
	m_browseServerButton = new QPushButton("...", serverPane);
	m_browseServerButton->setMaximumWidth(30);
	serverPathLayout->addWidget(m_serverPathEdit, 1);
	serverPathLayout->addWidget(m_browseServerButton);
	serverLayout->addLayout(serverPathLayout);

	m_serverEditor = new QTextEdit(serverPane);
	m_serverEditor->setTextFormat(Qt::PlainText);
	m_serverEditor->setFont(QFont("Courier New", 9));
	serverLayout->addWidget(m_serverEditor, 1);

	QWidget * sharedPane = new QWidget(topSplitter);
	QVBoxLayout * sharedLayout = new QVBoxLayout(sharedPane, 2, 2);
	QLabel * sharedLabel = new QLabel("Shared Template (.tpf)", sharedPane);
	sharedLayout->addWidget(sharedLabel);

	QHBoxLayout * sharedPathLayout = new QHBoxLayout(0, 0, 2);
	m_sharedPathEdit = new QLineEdit(sharedPane);
	m_browseSharedButton = new QPushButton("...", sharedPane);
	m_browseSharedButton->setMaximumWidth(30);
	sharedPathLayout->addWidget(m_sharedPathEdit, 1);
	sharedPathLayout->addWidget(m_browseSharedButton);
	sharedLayout->addLayout(sharedPathLayout);

	m_sharedEditor = new QTextEdit(sharedPane);
	m_sharedEditor->setTextFormat(Qt::PlainText);
	m_sharedEditor->setFont(QFont("Courier New", 9));
	sharedLayout->addWidget(m_sharedEditor, 1);

	mainLayout->addWidget(topSplitter, 1);

	// String file pane
	QGroupBox * stringGroup = new QGroupBox(1, Qt::Horizontal, "String File (.stf)", this);

	QWidget * stringWidget = new QWidget(stringGroup);
	QVBoxLayout * stringLayout = new QVBoxLayout(stringWidget, 2, 2);

	QHBoxLayout * stringPathLayout = new QHBoxLayout(0, 0, 2);
	m_stringPathEdit = new QLineEdit(stringWidget);
	m_browseStringButton = new QPushButton("...", stringWidget);
	m_browseStringButton->setMaximumWidth(30);
	stringPathLayout->addWidget(m_stringPathEdit, 1);
	stringPathLayout->addWidget(m_browseStringButton);
	stringLayout->addLayout(stringPathLayout);

	m_stringEditor = new QTextEdit(stringWidget);
	m_stringEditor->setTextFormat(Qt::PlainText);
	m_stringEditor->setFont(QFont("Courier New", 9));
	m_stringEditor->setMaximumHeight(150);
	stringLayout->addWidget(m_stringEditor, 1);

	mainLayout->addWidget(stringGroup);

	// Status and buttons
	m_statusLabel = new QLabel("Load a .tpf file or create a new template.", this);
	mainLayout->addWidget(m_statusLabel);

	QHBoxLayout * buttonLayout = new QHBoxLayout(0, 0, 6);
	buttonLayout->addStretch(1);
	m_saveButton = new QPushButton("Save TPF", this);
	m_compileButton = new QPushButton("Compile", this);
	m_compileButton->setEnabled(false);
	buttonLayout->addWidget(m_saveButton);
	buttonLayout->addWidget(m_compileButton);
	mainLayout->addLayout(buttonLayout);

	IGNORE_RETURN(connect(m_saveButton,          SIGNAL(clicked()), this, SLOT(onSave())));
	IGNORE_RETURN(connect(m_compileButton,        SIGNAL(clicked()), this, SLOT(onCompile())));
	IGNORE_RETURN(connect(m_browseServerButton,   SIGNAL(clicked()), this, SLOT(onBrowseServerPath())));
	IGNORE_RETURN(connect(m_browseSharedButton,   SIGNAL(clicked()), this, SLOT(onBrowseSharedPath())));
	IGNORE_RETURN(connect(m_browseStringButton,   SIGNAL(clicked()), this, SLOT(onBrowseStringPath())));
	IGNORE_RETURN(connect(m_newButton,            SIGNAL(clicked()), this, SLOT(onNewTemplate())));
}

// ----------------------------------------------------------------------

TemplateEditorWindow::~TemplateEditorWindow()
{
}

// ----------------------------------------------------------------------

void TemplateEditorWindow::loadTemplate(const std::string & tpfPath)
{
	std::ifstream file(tpfPath.c_str());
	if (!file.is_open())
	{
		QMessageBox::warning(this, "Template Editor", QString("Could not open: %1").arg(tpfPath.c_str()));
		return;
	}

	std::string contents;
	std::string line;
	while (std::getline(file, line))
	{
		contents += line;
		contents += "\n";
	}
	file.close();

	if (tpfPath.find("sys.server") != std::string::npos)
	{
		m_serverEditor->setText(contents.c_str());
		m_serverPathEdit->setText(tpfPath.c_str());
		m_currentServerPath = tpfPath;

		std::string sharedPath = tpfPath;
		std::string::size_type pos = sharedPath.find("sys.server");
		if (pos != std::string::npos)
			sharedPath.replace(pos, 10, "sys.shared");
		m_sharedPathEdit->setText(sharedPath.c_str());

		std::ifstream sharedFile(sharedPath.c_str());
		if (sharedFile.is_open())
		{
			std::string sharedContents;
			while (std::getline(sharedFile, line))
			{
				sharedContents += line;
				sharedContents += "\n";
			}
			sharedFile.close();
			m_sharedEditor->setText(sharedContents.c_str());
			m_currentSharedPath = sharedPath;
		}
	}
	else if (tpfPath.find("sys.shared") != std::string::npos)
	{
		m_sharedEditor->setText(contents.c_str());
		m_sharedPathEdit->setText(tpfPath.c_str());
		m_currentSharedPath = tpfPath;

		std::string serverPath = tpfPath;
		std::string::size_type pos = serverPath.find("sys.shared");
		if (pos != std::string::npos)
			serverPath.replace(pos, 10, "sys.server");
		m_serverPathEdit->setText(serverPath.c_str());

		std::ifstream serverFile(serverPath.c_str());
		if (serverFile.is_open())
		{
			std::string serverContents;
			while (std::getline(serverFile, line))
			{
				serverContents += line;
				serverContents += "\n";
			}
			serverFile.close();
			m_serverEditor->setText(serverContents.c_str());
			m_currentServerPath = serverPath;
		}
	}
	else
	{
		m_serverEditor->setText(contents.c_str());
		m_serverPathEdit->setText(tpfPath.c_str());
		m_currentServerPath = tpfPath;
	}

	m_hasLoadedTemplate = true;
	m_compileButton->setEnabled(true);
	m_statusLabel->setText(QString("Loaded: %1").arg(tpfPath.c_str()));
}

// ----------------------------------------------------------------------

void TemplateEditorWindow::newTemplate()
{
	m_serverEditor->clear();
	m_sharedEditor->clear();
	m_stringEditor->clear();
	m_serverPathEdit->clear();
	m_sharedPathEdit->clear();
	m_stringPathEdit->clear();
	m_currentServerPath.clear();
	m_currentSharedPath.clear();
	m_currentStringPath.clear();
	m_hasLoadedTemplate = false;
	m_compileButton->setEnabled(false);
	m_statusLabel->setText("New template - select a type and enter content.");
}

// ----------------------------------------------------------------------

void TemplateEditorWindow::onNewTemplate()
{
	std::string templateType = m_templateTypeCombo->currentText().latin1();

	newTemplate();

	std::string serverBoilerplate = getBoilerplate(templateType, "new_" + templateType, false);
	std::string sharedBoilerplate = getBoilerplate(templateType, "new_" + templateType, true);

	m_serverEditor->setText(serverBoilerplate.c_str());
	m_sharedEditor->setText(sharedBoilerplate.c_str());

	std::string basePath = "dsrc/sku.0/sys.server/compiled/game/object/" + templateType + "/";
	m_serverPathEdit->setText((basePath + "new_" + templateType + ".tpf").c_str());

	basePath = "dsrc/sku.0/sys.shared/compiled/game/object/" + templateType + "/";
	m_sharedPathEdit->setText((basePath + "new_" + templateType + ".tpf").c_str());

	m_stringPathEdit->setText(("dsrc/sku.0/sys.shared/compiled/game/string/en/object/" + templateType + "/new_" + templateType + ".stf").c_str());

	m_hasLoadedTemplate = true;
	m_compileButton->setEnabled(true);
	m_statusLabel->setText(QString("New %1 template created. Edit paths and content, then save.").arg(templateType.c_str()));
}

// ----------------------------------------------------------------------

std::string TemplateEditorWindow::getBoilerplate(const std::string & templateType, const std::string & templateName, bool isShared) const
{
	std::string result;

	if (isShared)
	{
		result += "@base object/";
		result += templateType;
		result += "/base/shared_";
		result += templateType;
		result += "_default.iff\n\n";

		result += "@class object_template_version [id] 0\n\n";

		result += "objectName = \"object/";
		result += templateType;
		result += "/";
		result += templateName;
		result += "\" \"";
		result += templateName;
		result += "\"\n";
		result += "detailedDescription = \"object/";
		result += templateType;
		result += "/";
		result += templateName;
		result += "\" \"";
		result += templateName;
		result += "_d\"\n";
		result += "lookAtText = \"object/";
		result += templateType;
		result += "/";
		result += templateName;
		result += "\" \"";
		result += templateName;
		result += "_l\"\n\n";

		result += "appearanceFilename = \"\"\n";
		result += "containerType = CT_none\n";
		result += "containerVolumeLimit = 0\n";
	}
	else
	{
		result += "@base object/";
		result += templateType;
		result += "/base/";
		result += templateType;
		result += "_default.iff\n\n";

		result += "@class object_template_version [id] 0\n\n";

		result += "// Server-side template for ";
		result += templateName;
		result += "\n";
		result += "scripts = []\n";
		result += "objvars = []\n";
	}

	return result;
}

// ----------------------------------------------------------------------

void TemplateEditorWindow::onSave()
{
	int confirm1 = QMessageBox::question(this, "Save Template",
		"Are you sure you want to save this template file?",
		QMessageBox::Yes, QMessageBox::No);

	if (confirm1 != QMessageBox::Yes)
		return;

	int confirm2 = QMessageBox::question(this, "Confirm Save",
		"This will overwrite the existing file. Are you absolutely sure?",
		QMessageBox::Yes, QMessageBox::No);

	if (confirm2 != QMessageBox::Yes)
		return;

	bool saved = false;

	std::string serverPath = m_serverPathEdit->text().latin1();
	if (!serverPath.empty())
	{
		std::string contents = m_serverEditor->text().latin1();
		if (saveToFile(serverPath, contents))
		{
			m_currentServerPath = serverPath;
			saved = true;
		}
	}

	std::string sharedPath = m_sharedPathEdit->text().latin1();
	if (!sharedPath.empty())
	{
		std::string contents = m_sharedEditor->text().latin1();
		if (saveToFile(sharedPath, contents))
		{
			m_currentSharedPath = sharedPath;
			saved = true;
		}
	}

	std::string stringPath = m_stringPathEdit->text().latin1();
	if (!stringPath.empty())
	{
		std::string contents = m_stringEditor->text().latin1();
		if (saveToFile(stringPath, contents))
		{
			m_currentStringPath = stringPath;
			saved = true;
		}
	}

	if (saved)
	{
		m_hasLoadedTemplate = true;
		m_compileButton->setEnabled(true);
		m_statusLabel->setText("Template saved successfully.");
		emit statusMessage("Template saved.");
	}
	else
	{
		m_statusLabel->setText("No files were saved. Check paths.");
	}
}

// ----------------------------------------------------------------------

void TemplateEditorWindow::onCompile()
{
	if (!m_hasLoadedTemplate)
	{
		QMessageBox::warning(this, "Template Editor", "No template loaded to compile.");
		return;
	}

	std::string tpfPath;
	if (!m_currentServerPath.empty())
		tpfPath = m_currentServerPath;
	else if (!m_currentSharedPath.empty())
		tpfPath = m_currentSharedPath;
	else
	{
		std::string p = m_serverPathEdit->text().latin1();
		if (!p.empty())
			tpfPath = p;
	}

	if (tpfPath.empty())
	{
		QMessageBox::warning(this, "Template Editor", "No template path available for compilation.");
		return;
	}

	m_statusLabel->setText(QString("Compiling: %1").arg(tpfPath.c_str()));

	if (runTemplateCompiler(tpfPath))
	{
		m_statusLabel->setText("Compilation successful.");
		emit statusMessage("Template compiled successfully.");
		MainFrame::getInstance().textToConsole(("TemplateEditor: Compiled " + tpfPath).c_str());
	}
	else
	{
		m_statusLabel->setText("Compilation failed. Check console for details.");
		QMessageBox::warning(this, "Template Editor", "TemplateCompiler failed. Check the console output.");
	}
}

// ----------------------------------------------------------------------

void TemplateEditorWindow::onBrowseServerPath()
{
	QString path = QFileDialog::getOpenFileName(QString::null, "Template Files (*.tpf);;All Files (*)", this, "browseServer", "Open Server Template");
	if (!path.isEmpty())
		loadTemplate(path.latin1());
}

// ----------------------------------------------------------------------

void TemplateEditorWindow::onBrowseSharedPath()
{
	QString path = QFileDialog::getOpenFileName(QString::null, "Template Files (*.tpf);;All Files (*)", this, "browseShared", "Open Shared Template");
	if (!path.isEmpty())
		loadTemplate(path.latin1());
}

// ----------------------------------------------------------------------

void TemplateEditorWindow::onBrowseStringPath()
{
	QString path = QFileDialog::getOpenFileName(QString::null, "String Files (*.stf);;All Files (*)", this, "browseString", "Open String File");
	if (!path.isEmpty())
	{
		m_stringPathEdit->setText(path);
		m_currentStringPath = path.latin1();

		std::ifstream file(m_currentStringPath.c_str());
		if (file.is_open())
		{
			std::string contents;
			std::string line;
			while (std::getline(file, line))
			{
				contents += line;
				contents += "\n";
			}
			file.close();
			m_stringEditor->setText(contents.c_str());
		}
	}
}

// ----------------------------------------------------------------------

bool TemplateEditorWindow::saveToFile(const std::string & path, const std::string & contents)
{
	FILE * fp = fopen(path.c_str(), "w");
	if (!fp)
		return false;

	fwrite(contents.c_str(), 1, contents.size(), fp);
	fclose(fp);
	return true;
}

// ----------------------------------------------------------------------

bool TemplateEditorWindow::runTemplateCompiler(const std::string & tpfPath)
{
	char command[2048];
	snprintf(command, sizeof(command), "TemplateCompiler -compile \"%s\"", tpfPath.c_str());

	int result = system(command);
	return (result == 0);
}

// ======================================================================
