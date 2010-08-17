#ifndef MAINWINDOWIMPL_H
#define MAINWINDOWIMPL_H
//
#include <QMainWindow>
#include <QFileDialog>
#include <QtGui>
#include "ui_mainwindow.h"
#include "filedata.h"

class MainWindowImpl : public QMainWindow, public Ui::MainWindow
{
	Q_OBJECT
	public:
		MainWindowImpl( QWidget * parent = 0, Qt::WFlags f = 0 );
		void SendArgs(int argc, char** argv);
		int SaveEditFields(TFileData* link);
		int LoadEditFields(TFileData* link);
		//int SaveNcFile(const TFileData *FileData, const QString FileName);
		//int LoadNcFile(TFileData *FileData, const QString FileName);
		
	private:
		//QList<QWidget*> editButtons;
		QList<QSuperEdit*> textFields;
		TFileData* file; //selected file
		
	private slots:
		void updateEditFields();
//		int Save();
		void addFile();
		void dublicateFile();
		void removeFile();
		
		void editStarted(QSuperEdit* widget);
		void editFinished(QSuperEdit* widget, bool ok);
		void spareButtonClicked(QSuperEdit* widget);
		
		void setHeatEnabled();
		void setPrintFreq(int i);
};

#endif




