#ifndef __QSUPEREDIT_H__
#define __QSUPEREDIT_H__

#include <QtGui>

class QSuperEdit : public QLineEdit
{
	Q_OBJECT
	Q_PROPERTY(bool spareButtonEnabled READ spareButtonEnabled WRITE setSpareButtonEnabled)
	Q_PROPERTY(bool editable READ editable WRITE setEditable)
	
	public:
		QSuperEdit(QWidget* parent=0, bool addSpareButton = 0);
		bool spareButtonEnabled() const {return SpareButton->isEnabled();};
		void setSpareButtonEnabled(bool b) { SpareButton->setEnabled(b); };
		bool editable() const {return EditButton->isEnabled();};
		void setEditable(bool b) { EditButton->setEnabled(b); };

	signals:
		void editStarted(QSuperEdit* widget);
		void editFinished(QSuperEdit* widget, bool ok);
		void spareButtonClicked(QSuperEdit* widget);
		
	protected:
		void keyPressEvent(QKeyEvent* event);

	private:
		QToolButton *EditButton;
		QToolButton *OkButton;
		QToolButton *SpareButton;
		QToolButton *CancelButton;
		
		QString backup;
		QString DisabledStyle;
		
	private slots:
		void edit();
		void finishOk();
		void finishCancel();
		void finishSpare();
};

#endif // __QSUPEREDIT_H__
