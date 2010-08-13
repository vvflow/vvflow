#ifndef __QDDLISTWIDGET_H__
#define __QDDLISTWIDGET_H__

#include <QtGui>
#include "filedata.h"

class QDDListWidget : public QListWidget
{
	Q_OBJECT
	public:
		QDDListWidget( QWidget * parent = 0): QListWidget(parent){fileList = new QList<TFileData*>;};
		QList<TFileData*> selected();
		void updateList();
		QListWidgetItem* add(TFileData* file);
		void removeAllSelected();

	protected:
		void startDrag(Qt::DropActions supportedActions);
		
		void dragEnterEvent(QDragEnterEvent *event); 
		void dragMoveEvent(QDragMoveEvent*){};
		void dropEvent(QDropEvent *event);
	
	private:
		QList<TFileData*> *fileList;
};

#endif // __QDDLISTWIDGET_H__
