#ifndef __FILEDATA_H__
#define __FILEDATA_H__

#include <QtGui>

class TFileData
{
	public: //constructors
		TFileData(){};
		TFileData(const QString sFileName);
		TFileData(const TFileData *sFileData);
		~TFileData(){};

	public: //variables
		QString FileName;
		
		double Re;
		double dt;
		
		QString InfSpeedX;
		QString InfSpeedY;
		QString Rotation;
		int BodyVorts;
		bool HeatEnabled;
		
		double TreeFarCriteria;
		double MinNodeSize;
		double MinG;
		double ConvEps;
		double MergeEps;
		
		int PrintFreq;
		QBitArray ValuesToSave;

	public: //functions
		int SaveToFile(const QString FileUrl);
		int LoadFromFile(const QString FileUrl);
};

#endif // __FILEDATA_H__
