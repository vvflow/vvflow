#include "qddlistwidget.h"
#include "iostream"
using namespace std;

QList<TFileData*> QDDListWidget::selected()
{
	QList<TFileData*> result;
	foreach(QListWidgetItem *item, selectedItems())
	{
		TFileData *link = (TFileData*)item->data(Qt::UserRole).value<void*>();
		result << link;
	}
	
	return result;
}

void QDDListWidget::updateList()
{
	clear();
	QListWidgetItem *item;
	
	foreach(TFileData *file, *fileList)
	{
		item = new QListWidgetItem(QIcon(":icons/file.svg"), file->FileName, this);
		item->setData(Qt::UserRole, qVariantFromValue((void*)file));
	}
}


QListWidgetItem* QDDListWidget::add(TFileData* file)
{
	QListWidgetItem *item;
	item = new QListWidgetItem(QIcon(":icons/file.svg"), file->FileName, this);
	item->setData(Qt::UserRole, qVariantFromValue((void*)file));
	fileList->append(file);
	
	return item;
}

void QDDListWidget::removeAllSelected()
{
	QList<TFileData*> selectedF(selected());
	QList<QListWidgetItem *> selectedI(selectedItems());

	foreach(QListWidgetItem* item, selectedI)
	{
		delete item;
	}
	
	foreach(TFileData* file, selectedF)
	{
		fileList->removeOne(file);
		delete file;
	}
}

////////////////////////// DRAG AND DROP //////////////////////////////////////

void QDDListWidget::startDrag( Qt::DropActions supportedActions)
{
	QMimeData *mimeData = new QMimeData;
	
	QList<TFileData*> Files(selected());
	QList<QUrl> UrlList;
	foreach (TFileData *file, Files)
	{
		QUrl FilePath = QUrl::fromLocalFile(QFileInfo(QDir::temp(), file->FileName).filePath());
		file->SaveToFile(FilePath.path());
		UrlList << FilePath;
		mimeData->setData("text/plain", FilePath.toString().toAscii());
	}
	
	mimeData->setUrls(UrlList);
	QDrag *drag = new QDrag(this);
	drag->setMimeData(mimeData);
	if (Files.size() > 1)
		drag->setPixmap(QPixmap(":icons/files.svg"));
	else
		drag->setPixmap(QPixmap(":icons/file.svg"));
	
	drag->exec(supportedActions);
}

void QDDListWidget::dragEnterEvent(QDragEnterEvent *event)
{
	QList<QUrl> UrlList(event->mimeData()->urls());
	QFileInfo FileInfo;
	
	if (!event->mimeData()->hasUrls() || (event->source() == this) )
	{
		//event->setDropAction(Qt::IgnoreAction);
		event->ignore();
		return;
	}

	
	for (int i=0; i<UrlList.size(); i++)
	{
		FileInfo = QFileInfo(UrlList[i].path());
		if (FileInfo.isFile() && FileInfo.isReadable())
		{
			event->setDropAction(Qt::MoveAction);
			event->accept();
			cout << "drop accepted" << endl;
			return;
		}
	}

	event->ignore();
}

void QDDListWidget::dropEvent(QDropEvent *event)
{
	setCurrentRow(-1);
	clearSelection();
	QList<QUrl> UrlList(event->mimeData()->urls());
	cout << event->mimeData()->data("text/uri-list").constData() << endl;
	for (int i=0; i<UrlList.size(); i++)
	{
		QString FileUrl = UrlList[i].path();
		cout << FileUrl.toAscii().constData() << endl;
		TFileData *NewFile = new TFileData();
		
		if (NewFile->LoadFromFile(FileUrl))
			delete NewFile;
		else
		{
			setCurrentItem(add(NewFile), QItemSelectionModel::Select);
		}
	}
}
