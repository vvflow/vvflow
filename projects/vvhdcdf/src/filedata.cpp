#include "filedata.h"
#include <netcdfcpp.h>
#include "iostream"
using namespace std;

TFileData::TFileData(const QString sFileName)
{
	FileName = sFileName;
	
	Re = 0;
	dt = 0;
	InfSpeedX.append("echo 1");
	BodyVorts = 0;
	HeatEnabled = false;
	
	TreeFarCriteria = 10;
	MinNodeSize = 0;
	MinG = 1E-7;
	ConvEps = 1E-8;
	MergeEps = 0.156;
	
	PrintFreq = 10;
}

TFileData::TFileData(const TFileData *sFileData)
{
	FileName = sFileData->FileName;
	
	Re = sFileData->Re;
	dt = sFileData->dt;
	InfSpeedX = sFileData->InfSpeedX;
	InfSpeedY = sFileData->InfSpeedY;
	Rotation = sFileData->Rotation;
	BodyVorts = sFileData->BodyVorts;
	HeatEnabled = sFileData->HeatEnabled;
	
	TreeFarCriteria = sFileData->TreeFarCriteria;
	MinNodeSize = sFileData->MinNodeSize;
	MinG = sFileData->MinG;
	ConvEps = sFileData->ConvEps;
	MergeEps = sFileData->MergeEps;
	
	PrintFreq = sFileData->PrintFreq;
}

///////////////////////////////////////////////////////////////////////////////

int TFileData::SaveToFile(const QString FileUrl)
{
	NcFile dataFile(FileUrl.toAscii(), NcFile::Replace);
	if (!dataFile.is_valid())
	{
		cout << "Couldn't open file!\n";
		return -1;
	}
	
	//NcDim* nDim = dataFile.add_dim("n");
	
	dataFile.add_att("Re", Re);
	dataFile.add_att("dt", dt);
	dataFile.add_att("InfSpeedX", InfSpeedX.toAscii().constData());
	dataFile.add_att("InfSpeedY", InfSpeedY.toAscii().constData());
	dataFile.add_att("Rotation", Rotation.toAscii().constData());
	dataFile.add_att("BodyVorts", BodyVorts);
	dataFile.add_att("HeatEnabled", HeatEnabled);
	dataFile.add_att("TreeFarCriteria", TreeFarCriteria);
	dataFile.add_att("MinNodeSize", MinNodeSize);
	dataFile.add_att("MinG", MinG);
	dataFile.add_att("ConvEps", ConvEps);
	dataFile.add_att("MergeEps", MergeEps);
	dataFile.add_att("PrintFreq", PrintFreq);

	return 0;
}

int TFileData::LoadFromFile(const QString FileUrl)
{
	NcFile dataFile(FileUrl.toAscii(), NcFile::ReadOnly);
	if (!dataFile.is_valid())
	{
		cout << "Couldn't open file!\n";
		return -1;
	}
	
	NcError err(NcError::verbose_nonfatal);
	
	FileName = QFileInfo(FileUrl).fileName();
	
	NcAtt* att;
	
	att = dataFile.get_att("Re"); if (att) Re = att->as_double(0);
	att = dataFile.get_att("dt");if (att) dt = att->as_double(0);
	att = dataFile.get_att("InfSpeedX");if (att) InfSpeedX = att->as_string(0);
	att = dataFile.get_att("InfSpeedY");if (att) InfSpeedY = att->as_string(0);
	att = dataFile.get_att("Rotation"); if (att) Rotation = att->as_string(0);
	att = dataFile.get_att("BodyVorts"); if (att) BodyVorts = att->as_int(0);
	att = dataFile.get_att("HeatEnabled"); if (att) HeatEnabled = att->as_ncbyte(0);
	att = dataFile.get_att("TreeFarCriteria"); if (att) TreeFarCriteria = att->as_double(0);
	att = dataFile.get_att("MinNodeSize"); if (att) MinNodeSize = att->as_double(0);
	att = dataFile.get_att("MinG"); if (att) MinG = att->as_double(0);
	att = dataFile.get_att("ConvEps"); if (att) ConvEps = att->as_double(0);
	att = dataFile.get_att("MergeEps"); if (att) MergeEps = att->as_double(0);
	att = dataFile.get_att("PrintFreq"); if (att) PrintFreq = att->as_int(0);

	return 0;
}
