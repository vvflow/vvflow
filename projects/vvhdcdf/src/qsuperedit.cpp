#include "qsuperedit.h"
#include "iostream"
using namespace std;

#define EnabledStyle "QLineEdit { padding-right: 75px; min-height: 27px; }"


QSuperEdit::QSuperEdit(QWidget* parent, bool addSpareButton)
	:QLineEdit(parent)
{
	setReadOnly(true);
	QColor c = style()->standardPalette().color(QPalette::Disabled, QPalette::Background);
	DisabledStyle = QString("QLineEdit { padding-right: 25px; min-height: 27px; \
										background-color: rgb(%1,%2,%3) }")
					.arg(c.red())
					.arg(c.green())
					.arg(c.blue());
	setStyleSheet(DisabledStyle);
	
	QHBoxLayout *Layout = new QHBoxLayout(this);
	Layout->setSpacing(0);
	Layout->setContentsMargins(0, 0, 0, 0);
	
	QSpacerItem *horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	
	#define InitButton(button) \
	button = new QToolButton(this); \
	button->setIconSize(QSize(16, 16)); \
	button->setMinimumSize(QSize(25,25)); \
	button->setMaximumSize(QSize(25,25)); \
	button->setCursor(Qt::ArrowCursor); \
	button->setFocusPolicy(Qt::NoFocus); \
	button->setStyleSheet("QToolButton:!hover { border: none; padding: 0px; }");
	
	InitButton(EditButton);
	InitButton(OkButton);
	InitButton(SpareButton);
	InitButton(CancelButton);
	
	#undef InitButton
	
	EditButton->setIcon(QIcon(":icons/edit16.png"));
	OkButton->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
	SpareButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogInfoView));
	CancelButton->setIcon(style()->standardIcon(QStyle::SP_DialogDiscardButton));
	
	EditButton->setVisible(true);
	OkButton->setVisible(false);
	SpareButton->setVisible(false);
	SpareButton->setEnabled(addSpareButton);
	CancelButton->setVisible(false);
	
	Layout->addItem(horizontalSpacer);
	Layout->addWidget(EditButton);
	Layout->addWidget(OkButton);
	Layout->addWidget(SpareButton);
	Layout->addWidget(CancelButton);
	
	//updateMargins();
	connect(EditButton, SIGNAL(clicked()), this, SLOT(edit()));
	connect(OkButton, SIGNAL(clicked()), this, SLOT(finishOk()));
	connect(CancelButton, SIGNAL(clicked()), this, SLOT(finishCancel()));
	connect(SpareButton, SIGNAL(clicked()), this, SLOT(finishSpare()));
}

void QSuperEdit::keyPressEvent(QKeyEvent* event)
{
	if (!isReadOnly())
	{
		if (event->key() == Qt::Key_Escape)
			finishCancel();
		if ((event->key() == Qt::Key_Return) || (event->key() == Qt::Key_Enter))
			finishOk();
	} else if (EditButton->isEnabled())
	{
		if ((event->key() == Qt::Key_Return) || (event->key() == Qt::Key_Enter))
			edit();
	}
	
	QLineEdit::keyPressEvent(event);
}

void QSuperEdit::edit()
{
	backup = text();
	EditButton->setVisible(false);
	OkButton->setVisible(true);
	SpareButton->setVisible(true);
	CancelButton->setVisible(true);
	
	setReadOnly(false);
	setStyleSheet(EnabledStyle);
	selectAll();
	emit editStarted(this);
}

void QSuperEdit::finishOk()
{
	EditButton->setVisible(true);
	OkButton->setVisible(false);
	SpareButton->setVisible(false);
	CancelButton->setVisible(false);
	
	setReadOnly(true);
	setStyleSheet(DisabledStyle);
	
	emit editFinished(this, true);
}

void QSuperEdit::finishCancel()
{
	setText(backup);
	EditButton->setVisible(true);
	OkButton->setVisible(false);
	SpareButton->setVisible(false);
	CancelButton->setVisible(false);
	
	setReadOnly(true);
	setStyleSheet(DisabledStyle);
	
	emit editFinished(this, false);
}

void QSuperEdit::finishSpare()
{
	emit spareButtonClicked(this);
}

