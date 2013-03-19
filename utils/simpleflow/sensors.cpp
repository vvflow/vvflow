#include "stdio.h"

#include "core.h"
#include "convectivefast.h"

class sensors
{
	public:
		sensors(Space* sS, convectivefast *sconv, const char* sensors_file, const char* output);
		void loadFile(const char* file);
		void output();

	private:
		Space *S;
		convectivefast *conv;
		FILE *fout;
		vector <TVec> *slist;
};

sensors::sensors(Space* sS, convectivefast *sconv, const char* sensors_file, const char* output)
{
	S = sS;
	conv = sconv;
	fout = NULL;
	slist = NULL;

	loadFile(sensors_file);
	if (slist->size_safe()) fout = fopen(output, "a");
}

void sensors::loadFile(const char* file)
{
	if (!file) return;

	FILE *fin = fopen(file, "r");
	if (!fin) { cerr << "No file called \'" << file << "\'\n"; return; }

	slist = new vector<TVec>();
	TVec vec(0, 0);
	while ( fscanf(fin, "%lf %lf", &vec.x, &vec.y)==2 )
	{
		slist->push_back(vec);
	}

	fclose(fin);
}

void sensors::output()
{
	if (!fout) return;

	fprintf(fout, "%lg", double(S->Time));
	const_for(slist, lvec)
	{
		TVec tmp = conv->SpeedSumFast(*lvec);
		fprintf(fout, " \t%lg \t%lg", tmp.x, tmp.y);
	}
	fprintf(fout, "\n");
	fflush(fout);
}
