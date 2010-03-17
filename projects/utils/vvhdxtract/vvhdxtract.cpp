#include "libVVHD/core.h" //ядро комплекса. Содержит базовые типы, в тч и дерево для быстрых модулей

//#include "libVVHD/convectivefast.h" //модуль быстрой конвекции
//#include "libVVHD/diffmergefast.h" //модуль, объединяющий диффузию и объединение вихрей
#define XML_ENABLE // вкрючаем xmlные фичи в utils.h
#include "libVVHD/utils.h" //утилиты для печати
//#include "libVVHD/flowmove.h" //модуль перемещения вихрей

#include "iostream"
#include "fstream"
#include <stdio.h>
#include <unistd.h>
#include "malloc.h"
#include "math.h"
#include "string.h"
//#include <pthread.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

using namespace std;

#define ERRORMESSAGE1 "Usage:\n\
\t vvhdxtract filename -r|-v|-p|-h [OPTIONS] \n\
\t-r   : request info about vorticity availability\n\
\t-v t : extract vorticity from timestep t\n\
\t-p A : extract parameter(s) A from all steps\n\
\t-h   : show this message and exit\n"

#define dbgout cerr

int main(int argc, char **argv)
{
//	for (int i=0; i<argc; i++) {cout << argv[i] << endl; }

	if (argc < 3) {dbgout << ERRORMESSAGE1 << endl; return -1; }
	if (argv[2][0] != '-') {dbgout << ERRORMESSAGE1 << endl; return -1; }
	if (argv[2][2]) {dbgout << ERRORMESSAGE1 << endl; return -1; }
	char mode = argv[2][1];

//	for (int i=0; i<argc; i++) {cout << argv[i] << endl; }
//	return 0;
	xmlDoc *doc = OpenStorage(argv[1], NULL);
	if (!doc) { dbgout << "Unable to open file \"" << argv[1] << "\"\n"; return -1; }

	if (mode == 'r')
	{
		if (argc != 3) { dbgout << "Syntax error. " << ERRORMESSAGE1 << endl; return -1; }
		xmlNode* step=GetLastStep(doc);
		double Time;
		while(step)
		{
			if (CheckStepParameter(step, "VortexField")) 
			{
				if (GetStepDouble(step, "Time", &Time)) Time = -1;
				cout << Time << " ";
			}
			step = GetPrevNode(step);
		}
		cout << endl;
	} else 
	if (mode == 'v')
	{
		if (argc != 4) { dbgout << "Syntax error. " << ERRORMESSAGE1 << endl; return -1; }
		xmlNode* step=GetLastStep(doc); while (!CheckStepParameter(step, "VortexField")) {step = GetPrevNode(step);}
		double TimeReq=-1, TimeTmp; sscanf(argv[3], "%lf", &TimeReq);
		if (TimeReq) while(step)
		{
			GetStepDouble(step, "Time", &TimeTmp);
			if (fabs(TimeTmp-TimeReq)<1E-6) break;
			step = GetPrevNode(step);
		}
		if (!step) { dbgout << "No info about VortexField found." << endl; return -1;}
		

		char *EncodedVorticity=NULL;
		if(GetStepString(step, "VortexField", &EncodedVorticity)) return -2;

		int len = strlen(EncodedVorticity);
		int lsize = len/4/sizeof(double);
		double *DecodedVorticity = (double*)Base64ToBin((const char*)EncodedVorticity, len);
		double *Pos = DecodedVorticity;
		for (int i=0; i<lsize; i++) 
		{
			cout << *Pos++ << "\t";
			cout << *Pos++ << "\t";
			cout << *Pos++;
			cout << endl;
		}
		free(DecodedVorticity);
		free(EncodedVorticity);
		return 0;
	} else
	if (mode == 'p')
	{
		for (int i=3; i<argc; i++) { cout << argv[i] << "\t"; }
		cout << endl;
		xmlNode* step=GetFirstStep(doc);
		while (step)
		{
			for (int i=3; i<argc; i++)
			{
				for ( xmlNode *n=step->children; n; n=n->next )
				{
					if ( (n->type == XML_ELEMENT_NODE) && !strcmp((const char*) n->name, argv[i]) ) 
					{
						cout << (const char*)n->children->content << "\t" << flush;
					}
				}
			}
			cout << endl;
			step = GetNextNode(step);
		}

	} else 
	{ dbgout << "Error. " << ERRORMESSAGE1 << endl; return -1; }

	CloseStorage(doc);

	return 0;











	//заводим переменные
	double DFI, DT, RE, TreeFarCriteria=10, TreeMinNodeSize, MinG=1E-8, ConvEps=1E-6, MergeSqEps, HeatEnabled=0;
	int PrintFrequency=10, BodyVortexes;
	fstream fout;
	char fname[64];
	pthread_t thread;

	if (argc<2) { cout << "No file specified. Use ./exe file.xml\n"; return -1; } //проверяем синтаксис запуска

	//xmlDoc *doc = OpenStorage(argv[1], NULL);
	if (!doc) { cout << "Unable to open or create file \"" << argv[1] << "\"\n"; return -1; }

	xmlNode *head = GetLastHeader(doc);
	if (!head)
	{
		cout << "I couldn't find any regime info... " << endl;
		return -1;
	} else
	{
		if (GetHeaderInt(head, "BodyVortexes", &BodyVortexes)) { cout << "Variable \"BodyVortexes\" isn't initialized\n"; return -1; }
		DFI = 6.283185308/BodyVortexes;
		if (GetHeaderDouble(head, "DT", &DT)) { cout << "Variable \"DT\" isn't initialized\n"; return -1; }
		if (GetHeaderDouble(head, "RE", &RE)) { cout << "Variable \"RE\" isn't initialized\n"; return -1; }
		GetHeaderDouble(head, "TreeFarCriteria", &TreeFarCriteria);
		if (GetHeaderDouble(head, "TreeMinNodeSize", &TreeMinNodeSize)) TreeMinNodeSize=6*DFI;
		GetHeaderDouble(head, "MinG", &MinG);
		GetHeaderDouble(head, "ConvEps", &ConvEps);
		if (GetHeaderDouble(head, "MergeSqEps", &MergeSqEps)) MergeSqEps = DFI*DFI*0.09;
		GetHeaderDouble(head, "HeatEnabled", &HeatEnabled);
		GetHeaderInt(head, "PrintFrequency", &PrintFrequency);
	} 

//	cout << "loading vorticity... " << flush;
//	LoadVorticityFromLastStep(S, doc);
//	cout << S->VortexList->size << " loaded.\n";

	xmlSaveFormatFileEnc(argv[1], doc, "UTF-8", 1);
	CloseStorage(doc);




	return 0;
}

