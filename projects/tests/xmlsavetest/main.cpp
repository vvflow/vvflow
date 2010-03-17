#include "libVVHD/core.h" //ядро комплекса. Содержит базовые типы, в тч и дерево для быстрых модулей

#define XML_ENABLE // вкрючаем xmlные фичи в utils.h
#include "libVVHD/utils.h" //утилиты для печати

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

using namespace std;

int main(int argc, char **argv)
{
	Space *S = new Space(true, true, false, NULL, NULL, NULL);
	S->LoadVorticityFromFile("test.vort");

	xmlDoc *doc = OpenStorage("test.xml", VERSION);
	if (!doc) { cout << "1 Unable to open or create file \"" << argv[1] << "\"\n"; return -1; }
	xmlNode *step = AppendStep(doc);
	SaveVorticityToStep(S, step);
	xmlSaveFormatFileEnc("test.xml", doc, "UTF-8", 1);
	CloseStorage(doc);

	S->VortexList->Clear();

	doc = OpenStorage("test.xml", VERSION);
	if (!doc) { cout << "2 Unable to open or create file \"" << argv[1] << "\"\n"; return -1; }
	LoadVorticityFromLastStep(S, doc);
	CloseStorage(doc);

	fstream fout;
	fout.open("test.out", ios::out);
	PrintVorticity(fout, S, false); //сама печать. Аргументы: в какой поток, вселенная, печатать ли скорости вихрей
	fout.close();

	return 0;
}

