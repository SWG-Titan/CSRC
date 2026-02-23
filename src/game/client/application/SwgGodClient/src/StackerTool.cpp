// ======================================================================
//
// StackerTool.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "StackerTool.h"
#include "StackerTool.moc"

#include "GodClientData.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qspinbox.h>

// ======================================================================

StackerTool::StackerTool(QWidget *parent, const char *name)
: BaseStackerTool(parent, name)
{
	IGNORE_RETURN(connect(m_stackButton, SIGNAL(clicked()), this, SLOT(onStack())));
}

// ======================================================================

StackerTool::~StackerTool()
{
}

// ======================================================================

void StackerTool::onStack()
{
	int const count = m_countSpinBox->value();
	int const orientationIndex = m_orientationCombo->currentItem();
	bool ok = false;
	real const distance = m_distanceEdit->text().toFloat(&ok);
	if (!ok || distance < 0)
	{
		m_distanceEdit->setText("0");
		return;
	}

	bool const useMeshExtent = m_useMeshExtentCheck->isChecked();
	GodClientData::getInstance().stackObjects(count, orientationIndex, distance, useMeshExtent);
}

// ======================================================================
