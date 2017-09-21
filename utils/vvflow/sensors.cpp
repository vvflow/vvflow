#include "stdio.h"

#include "core.h"
#include "MConvectiveFast.hpp"

class sensors
{
	public:
		sensors(Space* sS, convectivefast *sconv, const char* sensors_file, const char* output);
		~sensors();
		void loadFile(const char* file);
		void output();

	private:
		Space *S;
		convectivefast *conv;
		FILE *fout;
		vector <TVec> slist;
};

sensors::sensors(Space* sS, convectivefast *sconv, const char* sensors_file, const char* output)
{
	S = sS;
	conv = sconv;
	fout = NULL;

	loadFile(sensors_file);
	if (slist.size()) fout = fopen(output, "a");
}

sensors::~sensors()
{
	if (fout) fclose(fout);
	fout = NULL;
}

void sensors::loadFile(const char* file)
{
	if (!file) return;

	FILE *fin = fopen(file, "r");
	if (!fin) { std::cerr << "No file called \'" << file << "\'" << std::endl; return; }

	TVec vec(0, 0);
	while ( fscanf(fin, "%lf %lf", &vec.x, &vec.y)==2 )
	{
		slist.push_back(vec);
	}

	fclose(fin);
}

void sensors::output()
{
	if (!fout) return;

	fprintf(fout, "%lg", double(S->Time));
	for (TVec vec: slist)
	{
		TVec tmp = conv->SpeedSumFast(vec);
		fprintf(fout, " \t%lg \t%lg", tmp.x, tmp.y);
	}
	fprintf(fout, "\n");
	fflush(fout);
}
