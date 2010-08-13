#include "mainwindowimpl.h"
#include "stdio.h"
#include "iostream"
using namespace std;

MainWindowImpl::MainWindowImpl( QWidget * parent, Qt::WFlags f) 
	: QMainWindow(parent, f)
{
	setupUi(this);
	listWidget->clear();
	
	textFields << FileNameEdit << ReEdit << dtEdit <<
		InfSpeedXEdit << InfSpeedYEdit << RotationEdit << 
		BodyVortsEdit << TreeFarEdit << MinNodeSizeEdit <<
		MinGEdit << ConvEpsEdit << MergeSqEpsEdit;
	
	updateEditFields();
	
	QList<int> SplitterSizes = splitter->sizes();
	SplitterSizes[0] += 10000;
	//SplitterSizes[1] = 0;
	splitter->setSizes(SplitterSizes);
	
	//connect(SaveButton, 			SIGNAL(clicked()), 		this, SLOT(Save()));
	connect(AddPushButton, 			SIGNAL(clicked()), 		this, SLOT(addFile()));
	connect(DublicatePushButton, 	SIGNAL(clicked()), 		this, SLOT(dublicateFile()));
	connect(RemovePushButton, 		SIGNAL(clicked()), 		this, SLOT(removeFile()));
	
	connect(listWidget, SIGNAL(itemSelectionChanged()), this, SLOT(updateEditFields()));
	connect(listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), FileNameEdit, SLOT(edit()));

	foreach(QSuperEdit *w, textFields)
	{
		connect(w, SIGNAL(editStarted(QSuperEdit*)), this, SLOT(editStarted(QSuperEdit*)));
		connect(w, SIGNAL(editFinished(QSuperEdit*, bool)), this, SLOT(editFinished(QSuperEdit*, bool)));
		connect(w, SIGNAL(spareButtonClicked(QSuperEdit*)), this, SLOT(spareButtonClicked(QSuperEdit*)));
	}
	connect(HeatEnabledEdit, SIGNAL(clicked()), this, SLOT(setHeatEnabled()));
	connect(PrintFreqEdit, SIGNAL(valueChanged(int)), this, SLOT(setPrintFreq(int)));
}

void MainWindowImpl::updateEditFields()
{
	QList<TFileData*> Selection = listWidget->selected();
	RemovePushButton->setEnabled(Selection.size());
	DublicatePushButton->setEnabled(Selection.size());
	if (Selection.size() != 1)
	{
		//clear all edit fields
		file = NULL;
		HeatEnabledEdit->setEnabled(false);
		PrintFreqEdit->setEnabled(false);
		ValuesToSaveEditButton->setEnabled(false);
		
		foreach(QSuperEdit* w, textFields)
		{
			w->setEditable(false);
			w->setText("");
		}
		HeatEnabledEdit->setChecked(false);
		PrintFreqEdit->setValue(10);
	} else
	{
		HeatEnabledEdit->setEnabled(true);
		PrintFreqEdit->setEnabled(true);
		ValuesToSaveEditButton->setEnabled(true);
		
		QString S;
		file = Selection[0];
		
		foreach(QSuperEdit* w, textFields){ w->setEditable(true); }
		FileNameEdit->setText(file->FileName);
		ReEdit->setText(S.setNum(file->Re));
		dtEdit->setText(S.setNum(file->dt));
		InfSpeedXEdit->setText(file->InfSpeedX);
		InfSpeedYEdit->setText(file->InfSpeedY);
		RotationEdit->setText(file->Rotation);
		BodyVortsEdit->setText(S.setNum(file->BodyVorts));
		HeatEnabledEdit->setChecked(file->HeatEnabled);
		TreeFarEdit->setText(S.setNum(file->TreeFarCriteria));
		MinNodeSizeEdit->setText(S.setNum(file->MinNodeSize));
		MinGEdit->setText(S.setNum(file->MinG));
		ConvEpsEdit->setText(S.setNum(file->ConvEps));
		MergeSqEpsEdit->setText(S.setNum(file->MergeSqEps));
		PrintFreqEdit->setValue(file->PrintFreq);
	}
}

///////////////////////////////////////////////////////////////////////////////

/*int MainWindowImpl::Save()
{
	QListWidgetItem *item = listWidget->currentItem();
	if (!item)
		return -1;
	TFileData *link = (TFileData*)item->data(Qt::UserRole).value<void*>();
	SaveEditFields(link);
	
	QString FileName;
	FileName = QFileDialog::getSaveFileName(
		this,
		"Choose a file to save into",
		QString::null,
		QString::null);
	
	cout << "Selected file \"" << FileName.toAscii().data() << "\"" << endl;
	
	link->SaveToFile(FileName);
	
	return 0;
}*/

void MainWindowImpl::addFile()
{
	listWidget->setCurrentRow(-1);
	listWidget->clearSelection();
	TFileData *NewFile = new TFileData("NewFile.nc");
	listWidget->setCurrentItem(listWidget->add(NewFile), QItemSelectionModel::Select);
}

void MainWindowImpl::dublicateFile()
{
	QList<TFileData*> SelectedData(listWidget->selected());
	listWidget->clearSelection();
	listWidget->setCurrentRow(-1);
	foreach(TFileData* file, SelectedData)
	{
		TFileData* NewFile = new TFileData(file);
		listWidget->setCurrentItem(listWidget->add(NewFile));
	}
}

void MainWindowImpl::removeFile()
{
	listWidget->removeAllSelected();
	listWidget->clearSelection();
	listWidget->setCurrentRow(-1);
}

///////////////////////////////////////////////////////////////////////////////

#define EnableAllButOne(b, one) \
	foreach(QSuperEdit* w, textFields) { if (w!=one) w->setEditable(b); } \
	HeatEnabledEdit->setEnabled(b); \
	PrintFreqEdit->setEnabled(b); \
	ValuesToSaveEditButton->setEnabled(b); \
	listWidget->setEnabled(b); \
	AddPushButton->setEnabled(b); \
	DublicatePushButton->setEnabled(b); \
	RemovePushButton->setEnabled(b);

void MainWindowImpl::editStarted(QSuperEdit* widget)
{
	//disabling all other widgets
	EnableAllButOne(false, widget);
}


void MainWindowImpl::editFinished(QSuperEdit* widget, bool ok)
{
	EnableAllButOne(true, widget);
	bool converted = true;
	if (ok)
	{
		if (widget == FileNameEdit)
		{
			file->FileName = FileNameEdit->text();
			listWidget->currentItem()->setText(file->FileName);
		} else if (widget == ReEdit)
			file->Re = ReEdit->text().toDouble(&converted);
		else if (widget == dtEdit)
			file->dt = dtEdit->text().toDouble(&converted);
		else if (widget == InfSpeedXEdit)
			file->InfSpeedX = InfSpeedXEdit->text();
		else if (widget == InfSpeedYEdit)
			file->InfSpeedY = InfSpeedYEdit->text();
		else if (widget == RotationEdit)
			file->Rotation = RotationEdit->text();
		else if (widget == BodyVortsEdit)
			file->BodyVorts = BodyVortsEdit->text().toInt(&converted);
		else if (widget == TreeFarEdit)
			file->TreeFarCriteria = TreeFarEdit->text().toDouble(&converted);
		else if (widget == MinNodeSizeEdit)
			file->MinNodeSize = MinNodeSizeEdit->text().toDouble(&converted);
		else if (widget == MinGEdit)
			file->MinG = MinGEdit->text().toDouble(&converted);
		else if (widget == ConvEpsEdit)
			file->ConvEps = ConvEpsEdit->text().toDouble(&converted);
		else if (widget == MergeSqEpsEdit)
			file->MergeSqEps = MergeSqEpsEdit->text().toDouble(&converted);
		
		
		if (!converted) widget->setText("0");
	}
}

void MainWindowImpl::spareButtonClicked(QSuperEdit* widget)
{
	if (widget == MinNodeSizeEdit)
	{
		bool ok;
		double x = QInputDialog::getDouble(this, tr("dfi multiplier"), 
								tr("multiplier = "), 2., 0., 100., 2, &ok);
		
		if (ok) 
		{
			double MinNodeSize = x * 6.283185308/file->BodyVorts;
			QString S; 
			widget->setText(S.setNum(MinNodeSize));
		}
	} else if (widget == MergeSqEpsEdit)
	{
		bool ok;
		double x = QInputDialog::getDouble(this, tr("dfi multiplier"), 
								tr("multiplier = "), 0, 0.3, 1, 2, &ok);
		
		if (ok) 
		{
			double MergeEps = x * 6.283185308/file->BodyVorts;
			QString S; 
			widget->setText(S.setNum(MergeEps*MergeEps));
		}
	}
}

#undef EnableAllButOne

void MainWindowImpl::setHeatEnabled()
{
	file->HeatEnabled = HeatEnabledEdit->isChecked();
}

void MainWindowImpl::setPrintFreq(int i)
{
	if (file)
		file->PrintFreq = i;
}

