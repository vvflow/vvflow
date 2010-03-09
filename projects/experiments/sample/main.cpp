#include "libVVHD/core.h" //ядро комплекса. Содержит базовые типы, в тч и дерево для быстрых модулей

#include "libVVHD/convectivefast.h" //модуль быстрой конвекции
#include "libVVHD/diffmergefast.h" //модуль, объединяющий диффузию и объединение вихрей
#include "libVVHD/utils.h" //утилиты для печати
#include "libVVHD/flowmove.h" //модуль перемещения вихрей

#include "iostream"
#include "fstream"
#include <stdio.h>
#include <unistd.h>
//#include <pthread.h>

#define BodyVortexes 500 //число присоединенных вихрей
#define RE 300 //Рейнольдс по радиусу!!!! не путать! во всех статьях принято указывать по диаметру
#define DT 0.05 //шаг по времени
#define print 10 //частота печати
#define DFI 6.28/BodyVortexes //вспомогательная переменная

using namespace std;

double InfSpeedX(double t) //функция скорости на бесконечности
{
	return 1;
}

void * diff (void* args) // параллельная нить для диффузии
{
	CalcVortexDiffMergeFast();
	return NULL;
}

int main()
{
	Space *S = new Space(true, true, false, InfSpeedX, NULL, NULL); //создаем вселенную. Аргументы: есть ли в ней вихри, тело, тепло; ссылки на функции: скорость X,Y на бесконечности, скорость вращения цилиндра
	S->ConstructCircle(BodyVortexes); //создаем во вселенной круг
	InitTree(S, 10, 4*DFI); //инициализируем дерево. Аргументы: ссылка на вселенную, критерий дальности ячеек (это отдельный разговор), минимальный размер ячейки
	InitFlowMove(S, DT, 1E-6); //инициализируем модуль перемещения. Аргументы: о5 вселенная, шаг по времени, критерий циркуляции (если модуль циркуляции вихря меньше - удаляем)
	InitConvectiveFast(S, 1E-4); //инициализируем модуль конвекции. Аргументы: вселенная, радиус дискретности
	InitDiffMergeFast(S, RE, DFI*DFI*0.16); // инициализируем диффузию и объединение. Аргументы: вселенная, Рейнольдс, критерий объединения (если вихри сближаются - объединяем)

	//затираем файл с силами
	fstream fout;
	char fname[64];
	sprintf(fname, "Forces");
	fout.open(fname, ios::out);
	fout.close();
	pthread_t thread;

	//здесь возможна загрузка из файла
	//S->LoadVorticityFromFile("restart_2000.vort");
	//S->Time = 100; //время с которого загружаемся
	//не забыть заменить число i
	//в загружаемом файле условие непротекание должно выполняться

	//запускаем основной цикл
	for (int i=0+1; i<100000; i++)
	{
		//считаем скорости
		BuildTree(true, false, false); //строим дерево. Аргументы - вносить ли в дерево информацию о: вихрях, теле, тепле
		pthread_create(&thread, NULL, &diff, NULL); //запускаем нить диффузии
		CalcConvectiveFast(); //параллельно вычисляем конвективную скорость
		pthread_join(thread, NULL); //присоединяем нить диффузии
		DestroyTree(); //убиваем дерево

		//двигаем вихри
		MoveAndClean(true); //интегрирование скоростей и удаление слишком маленьких. Аргументы: удалять ли вихри, проникшие внутрь цилиндра.

		//корректируем условие прилипания, которое нарушилось после перемещения
		BuildTree(true, true, false);
		CalcCirculationFast(); //считаем циркуляции присоединенных вихрей
		DestroyTree();

		VortexShed(); //отпускаем присоединенные вихри в свободный полет

		//шаг завершен

		//раз в print шагов печатаем вихри
		if (!(i%print)) 
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
	}



	return 0;
}

