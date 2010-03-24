#include "libVVHD/core.h" //ядро комплекса. Содержит базовые типы, в тч и дерево для быстрых модулей

#include "libVVHD/convectivefast.h" //модуль быстрой конвекции
#include "libVVHD/diffusivefast.h"
#include "libVVHD/mergefast.h"
//#include "libVVHD/diffmergefast.h" //модуль, объединяющий диффузию и объединение вихрей
#define XML_ENABLE // вкрючаем xmlные фичи в utils.h
#include "libVVHD/utils.h" //утилиты для печати
#include "libVVHD/utils_xml.h" //утилиты для печати
#include "libVVHD/flowmove.h" //модуль перемещения вихрей

#include "iostream"
#include "fstream"
#include <stdio.h>
#include <unistd.h>
#include "malloc.h"
#include "string.h"
//#include <pthread.h>
#include <libxml/parser.h>
#include <libxml/tree.h>


#define VERSION "VVHD cyl v0.1a"
/*
#define BodyVortexes 500 //число присоединенных вихрей
#define RE 300 //Рейнольдс по радиусу!!!! не путать! во всех статьях принято указывать по диаметру
#define DT 0.05 //шаг по времени
#define print 10 //частота печати
#define DFI 6.28/BodyVortexes //вспомогательная переменная
*/

using namespace std;



/****************************************************************/

char *InfSpeedXsh; //команда на баше, вычисляющая скорость набегающего потока //пример: "echo 1", либо "echo s($t)+1 | bc -l"
char *InfSpeedYsh; //аналогично для Vy
char *Rotationsh; //аналогично для вращения цилиндра (скорость поверхности)
char *ExecOnStepFinishSh; // команда которая будет выполняться в конце каждого шага. Удобно для автоматического построения картинок

double InfSpeedX(double t) 
{
	if (!InfSpeedXsh) return 0;
	double result;
	char *exec; exec = (char*)(malloc(strlen(InfSpeedXsh)+32));
	sprintf(exec, "t=%lf; T=%lf; %s", t, t, InfSpeedXsh);

	FILE *pipe = popen(exec,"r");
	if (!pipe) return 0;
	if (!fscanf(pipe, "%lf", &result)) result=0;
	pclose(pipe);

	return result;
}

double InfSpeedY(double t)
{
	if (!InfSpeedYsh) return 0;
	double result;
	char *exec; exec = (char*)(malloc(strlen(InfSpeedYsh)+32));
	sprintf(exec, "t=%lf; T=%lf; %s", t, t, InfSpeedYsh);

	FILE *pipe = popen(exec,"r");
	if (!pipe) return 0;
	if (!fscanf(pipe, "%lf", &result)) result=0;
	pclose(pipe);

	return result;
}

double Rotation(double t) //функция вращения цилиндра
{
	if (!Rotationsh) return 0;
	double result;
	char *exec; exec = (char*)(malloc(strlen(Rotationsh)+32));
	sprintf(exec, "t=%lf; T=%lf; %s", t, t, Rotationsh);

	FILE *pipe = popen(exec,"r");
	if (!pipe) return 0;
	if (!fscanf(pipe, "%lf", &result)) result=0;
	pclose(pipe);

	return result;
}

void ExecOnStepFinish(double Time, double InfSpeedX, double InfSpeedY, 
					double BodyRotation, double BodyAngle, int VortexListSize, double ForceX, double ForceY,
					int cleaned, int merged, double GSumm, double Integral)
{
	if (!ExecOnStepFinishSh) return;
	char *c = ExecOnStepFinishSh;
	char VarName[64], tmp[128];
	char exec[512]="";
	while (c)
	{
		if (*c=='$')
		{
			int i=0;
			while (*c!=' ')
			{
				VarName[i] = *c;
				i++; c++;
			}
			VarName[i] = 0;
			if (!strcmp(VarName, "Time")) { sprintf(tmp, "Time=%lf; ", Time); strcat(exec, tmp); } else
			if (!strcmp(VarName, "BodyAngle")) { sprintf(tmp, "BodyAngle=%lf; ", BodyAngle); strcat(exec, tmp); } else
			if (!strcmp(VarName, "InfSpeedX")) { sprintf(tmp, "InfSpeedX=%lf; ", InfSpeedX); strcat(exec, tmp); } else
			if (!strcmp(VarName, "InfSpeedY")) { sprintf(tmp, "InfSpeedY=%lf; ", InfSpeedY); strcat(exec, tmp); } else
			if (!strcmp(VarName, "BodyRotation")) { sprintf(tmp, "BodyRotation=%lf; ", BodyRotation); strcat(exec, tmp); } else
			if (!strcmp(VarName, "BodyAngle")) { sprintf(tmp, "BodyAngle=%lf; ", BodyAngle); strcat(exec, tmp); } else
			if (!strcmp(VarName, "VortexListSize")) { sprintf(tmp, "VortexListSize=%d; ", VortexListSize); strcat(exec, tmp); } else
			if (!strcmp(VarName, "ForceX")) { sprintf(tmp, "ForceX=%lf; ", ForceX); strcat(exec, tmp); } else
			if (!strcmp(VarName, "ForceY")) { sprintf(tmp, "ForceY=%lf; ", ForceY); strcat(exec, tmp); } else
			if (!strcmp(VarName, "cleaned")) { sprintf(tmp, "cleaned=%d; ", cleaned); strcat(exec, tmp); } else
			if (!strcmp(VarName, "merged")) { sprintf(tmp, "merged=%d; ", merged); strcat(exec, tmp); } else
			if (!strcmp(VarName, "GSumm")) { sprintf(tmp, "GSumm=%lf; ", GSumm); strcat(exec, tmp); } else
			if (!strcmp(VarName, "Integral")) { sprintf(tmp, "Integral=%lf; ", Integral); strcat(exec, tmp); }
		}
		c++;
	}
	strcat(exec, ExecOnStepFinishSh);
	FILE *pipe = popen(exec,"r"); 
	if (!pipe) return;
	pclose(pipe);
	return;
}

/***************************************************************/

void * diff (void* args) // параллельная нить для диффузии
{
	//CalcVortexDiffMergeFast();
	CalcVortexDiffusiveFast();
	return NULL;
}

int main(int argc, char **argv)
{
	//заводим переменные
	double DFI, DT, RE, TreeFarCriteria=10, TreeMinNodeSize, MinG=1E-8, ConvEps=1E-6, MergeSqEps, HeatEnabled=0;
	double InfSpeedXVal, InfSpeedYVal, RotationVal;
	int PrintFrequency=10, BodyVortexes;
	fstream fout;
	char fname[64];
	pthread_t thread;

	if (argc<2) { cout << "No file specified. Use ./exe file.xml\n"; return -1; } //проверяем синтаксис запуска

	xmlDoc *doc = OpenStorage(argv[1], VERSION);
	if (!doc) { cout << "Unable to open or create file \"" << argv[1] << "\"\n"; return -1; }

	xmlNode *head = GetLastHeader(doc);
	if (!head)
	{
		cout << "I couldn't find any regime info... " << endl;
		return -1;
	} else
	{
		InfSpeedXsh = NULL; GetHeaderString(head, "InfSpeedXsh", &InfSpeedXsh);
		InfSpeedYsh = NULL; GetHeaderString(head, "InfSpeedYsh", &InfSpeedYsh);
		Rotationsh = NULL; GetHeaderString(head, "Rotationsh", &Rotationsh);
		ExecOnStepFinishSh = NULL; GetHeaderString(head, "ExecOnStepFinish", &ExecOnStepFinishSh);
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

	Space *S = new Space(true, true, false, InfSpeedX, InfSpeedY, Rotation); //создаем вселенную. Аргументы: есть ли в ней вихри, тело, тепло; ссылки на функции: скорость X,Y на бесконечности, скорость вращения (поверхности) цилиндра
	S->ConstructCircle(BodyVortexes); //создаем во вселенной круг
	InitTree(S, TreeFarCriteria, TreeMinNodeSize); //инициализируем дерево. Аргументы: ссылка на вселенную, критерий дальности ячеек (это отдельный разговор), минимальный размер ячейки
	InitFlowMove(S, DT, MinG); //инициализируем модуль перемещения. Аргументы: о5 вселенная, шаг по времени, критерий циркуляции (если модуль циркуляции вихря меньше - удаляем)
	InitConvectiveFast(S, ConvEps); //инициализируем модуль конвекции. Аргументы: вселенная, радиус дискретности
	//InitDiffMergeFast(S, RE, MergeSqEps); // инициализируем диффузию и объединение. Аргументы: вселенная, Рейнольдс, критерий объединения (если вихри сближаются - объединяем)
	InitDiffusiveFast(S, RE);
	InitMergeFast(S, MergeSqEps);


	//затираем файл с силами
	/*sprintf(fname, "Forces");
	fout.open(fname, ios::out);
	fout.close();*/

	//здесь возможна загрузка из файла
	//S->LoadVorticityFromFile("restart_2000.vort");
	//S->Time = 100; //время с которого загружаемся
	//в загружаемом файле условие непротекание должно выполняться

	cout << "loading vorticity... " << flush;
	LoadVorticityFromLastStep(S, doc);
	cout << S->VortexList->size << " loaded from T=" << S->Time << endl;

	xmlSaveFormatFileEnc(argv[1], doc, "UTF-8", 1);
	CloseStorage(doc);

	//запускаем основной цикл
	for (int i=1; ; i++)
	{
		InfSpeedXVal = InfSpeedX(S->Time);
		InfSpeedYVal = InfSpeedY(S->Time);
		RotationVal = Rotation(S->Time);

		//считаем скорости
		BuildTree(true, false, false); //строим дерево. Аргументы - вносить ли в дерево информацию о: вихрях, теле, тепле
		MergeFast();
		pthread_create(&thread, NULL, &diff, NULL); //запускаем нить диффузии
		CalcConvectiveFast(); //параллельно вычисляем конвективную скорость
		pthread_join(thread, NULL); //присоединяем нить диффузии
		DestroyTree(); //убиваем дерево

		//двигаем вихри
		MoveAndClean(true); //интегрирование скоростей и удаление слишком маленьких. Аргументы: удалять ли вихри, проникшие внутрь цилиндра.
		int cleaned = CleanedV_toosmall()+CleanedV_inbody();
		//корректируем условие прилипания, которое нарушилось после перемещения
		BuildTree(true, true, false);
		CalcCirculationFast(); //считаем циркуляции присоединенных вихрей
		DestroyTree();

		VortexShed(); //отпускаем присоединенные вихри в свободный полет

		//шаг завершен

		doc = OpenStorage(argv[1], VERSION);
		if (doc)
		{
			xmlNode *step = AppendStep(doc);
			AppendNodeDouble(step, "Time", S->Time);
			if (Rotationsh) AppendNodeDouble(step, "BodyAngle", S->Angle);
//			if (InfSpeedXsh) AppendNodeDouble(step, "BodyX", S->BodyX);
//			if (InfSpeedYsh) AppendNodeDouble(step, "BodyY", S->BodyY);
			if (InfSpeedXsh) AppendNodeDouble(step, "InfSpeedX", InfSpeedXVal);
			if (InfSpeedYsh) AppendNodeDouble(step, "InfSpeedY", InfSpeedYVal);
			if (Rotationsh) AppendNodeDouble(step, "BodyRotation", RotationVal);
			AppendNodeInt(step, "VortexListSize", S->VortexList->size);
			AppendNodeDouble(step, "ForceX", S->ForceX/DT);
			AppendNodeDouble(step, "ForceY", S->ForceY/DT);
			S->ForceX = S->ForceY = 0; //зануляем силы, что бы старая информация не накапливалась
			AppendNodeInt(step, "cleaned", cleaned);
			//AppendNodeInt(step, "merged", DiffMergedFastV());
			AppendNodeInt(step, "merged", MergedFastV());
			AppendNodeDouble(step, "GSumm", S->gsumm()); //сумма всех вихрей в пространстве. Для невращающегося цилиндра должна мало отличаться от 0
			AppendNodeDouble(step, "Integral", S->Integral()); //функция пространства: сумма((r^2)*g)

			if (!(i%PrintFrequency)) { SaveVorticityToStep(S, step); }
		}

		xmlSaveFormatFileEnc(argv[1], doc, "UTF-8", 1);
		CloseStorage(doc);

		ExecOnStepFinish(S->Time, InfSpeedXVal, InfSpeedYVal, RotationVal, S->Angle, S->VortexList->size, 0, 0,
					cleaned, MergedFastV(), 0, 0);
		cout << "step " <<  i << " done. \t" << S->VortexList->size << "\n" ;

		/*
		//раз в print шагов печатаем вихри
		if (!(i%PrintFrequency)) 
		{
			
			sprintf(fname, "results/data%06d.vort", i); //куда печатать
			fout.open(fname, ios::out);
			PrintVorticity(fout, S, false); //сама печать. Аргументы: в какой поток, вселенная, печатать ли скорости вихрей
			fout.close();
			cout << "Printing \"" << fname << "\" finished. \n"; 
		}

		//каждый шаг печатаем силы
			sprintf(fname, "Forces");
			fout.open(fname, ios::out | ios::app);
			fout << S->Time << "\t" << S->ForceX/DT << "\t" << S->ForceY/DT << endl; //печатаем
			S->ForceX = S->ForceY = 0; //зануляем, что бы старая информация не накапливалась
			fout.close();

		//печатаем информацию о завершенном шаге
		cout << "step " << i << " done. \t" 
			<< CleanedV_toosmall()+CleanedV_inbody() << "v cleaned. \t" //сколько вихрей было удалено
			<< DiffMergedFastV() << "v merged. \t" << S->VortexList->size << " vortexes.\t" //сколько объединено
			<< "Gsumm=" << S->gsumm() << "\t" //сумма всех вихрей в пространстве. Для невращающегося цилиндра должна мало отличаться от 0
			<< "I=" << S->Integral() << endl; //функция пространства: сумма((р^2)*г)
		*/
	}



	return 0;
}

