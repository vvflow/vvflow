#include "MConvectiveFast.hpp"

// #include <cstdio>
#include <vector>

using std::vector;

class sensors
{
	public:
		sensors() = delete;
		sensors(Space* S, MConvectiveFast *conv, const char* sensors_file, const char* output);
		sensors(const sensors&) = delete;
		sensors& operator=(const sensors&) = delete;
		~sensors();
		void loadFile(const char* file);
		void output();

	private:
		Space *S;
		MConvectiveFast *conv;
		FILE *fout;
		vector <TVec> slist;
};

sensors::sensors(Space* S, MConvectiveFast *conv, const char* sensors_file, const char* output):
	S(S),
	conv(conv),
	fout(nullptr),
	slist()
{
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

	fprintf(fout, "%lg", double(S->time));
	for (TVec vec: slist)
	{
		TVec tmp = conv->velocity(vec);
		fprintf(fout, " \t%lg \t%lg", tmp.x, tmp.y);
	}
	fprintf(fout, "\n");
	fflush(fout);
}
