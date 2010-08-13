#ifndef VALUESDIALOGIMPL_H
#define VALUESDIALOGIMPL_H
//
#include <QDialog>
#include "ui_valuesdialog.h"
//
class ValuesDialogImpl : public QDialog, public Ui::ValuesDialog
{
Q_OBJECT
public:
	ValuesDialogImpl( QWidget * parent = 0, Qt::WFlags f = 0 );
private slots:
};
#endif





