// ======================================================================
//
// TemplateEditorWindow.h
// copyright 2024 Sony Online Entertainment
//
// Qt window for creating/editing template files (.tpf) and compiling
// with TemplateCompiler. Three panes: sys.server (top-left),
// sys.shared (top-right), string file (bottom-left). Save and Compile
// buttons with double-confirm on save. Supports new template creation
// with type-based boilerplate.
//
// ======================================================================

#ifndef INCLUDED_TemplateEditorWindow_H
#define INCLUDED_TemplateEditorWindow_H

// ======================================================================

#include <qdialog.h>
#include <string>

class QTextEdit;
class QPushButton;
class QLabel;
class QSplitter;
class QLineEdit;
class QComboBox;

// ======================================================================

class TemplateEditorWindow : public QDialog
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762

public:

	explicit TemplateEditorWindow(QWidget * parent = 0, const char * name = 0);
	virtual ~TemplateEditorWindow();

	void loadTemplate(const std::string & tpfPath);
	void newTemplate();

signals:
	void statusMessage(const char * msg);

public slots:
	void onSave();
	void onCompile();
	void onBrowseServerPath();
	void onBrowseSharedPath();
	void onBrowseStringPath();
	void onNewTemplate();

private:
	TemplateEditorWindow(const TemplateEditorWindow &);
	TemplateEditorWindow & operator=(const TemplateEditorWindow &);

	bool saveToFile(const std::string & path, const std::string & contents);
	bool runTemplateCompiler(const std::string & tpfPath);
	std::string getBoilerplate(const std::string & templateType, const std::string & templateName, bool isShared) const;

	QTextEdit *   m_serverEditor;
	QTextEdit *   m_sharedEditor;
	QTextEdit *   m_stringEditor;
	QLineEdit *   m_serverPathEdit;
	QLineEdit *   m_sharedPathEdit;
	QLineEdit *   m_stringPathEdit;
	QPushButton * m_saveButton;
	QPushButton * m_compileButton;
	QPushButton * m_newButton;
	QPushButton * m_browseServerButton;
	QPushButton * m_browseSharedButton;
	QPushButton * m_browseStringButton;
	QComboBox *   m_templateTypeCombo;
	QLabel *      m_statusLabel;

	std::string   m_currentServerPath;
	std::string   m_currentSharedPath;
	std::string   m_currentStringPath;
	bool          m_hasLoadedTemplate;
};

// ======================================================================

#endif
