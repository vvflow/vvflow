#include "stdio.h"
#include "stdlib.h"
#include "float.h"
#include "iostream"
using namespace std;

typedef const char TValues;
namespace val
{
	TValues Cp = 1;
	TValues Fr = 2;
	TValues Nu = 4;
}

inline double max(double a, double b) { return(a>b)?a:b; }
inline double min(double a, double b) { return(a<b)?a:b; }
inline double abs(double x) {return (x>0)?x:-x; }

int main(int argc, char** argv)
{
	if (argc<2)
	{
		fprintf(stderr, "Bad args. I require: %s filename [t_min t_max {p | f | n}]\n", argv[0]);
		fprintf(stderr, "p: pressure (Cp)\n f: friction (Fr)\nn: Nusselt (Nu)\n");
	}

	FILE* fin = fopen(argv[1], "rb");
	if (!fin) {perror("Cannot open file"); return -2;}
	int32_t vals; fread(&vals, 4, 1, fin);
	int32_t N; fread(&N, 4, 1, fin);

	int Cp = 1;//(vals & val::Cp)?1:0; //remove it when you dont plot bugged results
	int Fr = 1;//(vals & val::Fr)?1:0; //bug is already fixed, but results arent
	int Nu = 1;//(vals & val::Nu)?1:0;

	if (argc != 5)
	{ 
		if (argc >= 2)
		{
			fprintf(stderr, "Body segments: %d\n", N);
			fprintf(stderr, "Available values: %c%c%c\n", Cp?'p':'-', Nu?'n':'-', Fr?'f':'-');
			fprintf(stderr, "Available time intervals:\n");

			int count=0;
			float tmp, dt=0, start=-FLT_MAX, prev=-FLT_MAX/2;
			while ( fread(&tmp, 4, 1, fin)==1)
			{
				if (!dt && (start!=prev)) start = tmp; else
				if (start == prev) dt = tmp-prev; else
				if (abs(prev+dt-tmp)>0.1*dt)
				{
					fprintf(stderr, "[%g, %g] / %g; \n", start, prev, dt);
					start = tmp;
					dt = 0;
				}

				prev = tmp;
				fseek(fin, N*4*(2+Cp+Fr+Nu), SEEK_CUR);
				count++;
			}
			fprintf(stderr, "[%g, %g] / %g; \n", start, prev, dt);
			fprintf(stderr, "Total records: %d\n", count);

			fseek(fin, 8, SEEK_SET);
		
		}
	return -1;
	}

	float t_min = atof(argv[2]);
	float t_max = atof(argv[3]);
	char val = *argv[4];

	int I = ((val=='p')&&Cp)?Cp:((val=='f')&&Fr)?(Fr+Cp):((val=='n')&&Nu)?(Cp+Fr+Nu):(0);
	int count=0;
	if (!I) { fprintf(stderr, "There's no such value, \'%c\'\n", val); return -3; }

	float x[N], y[N], z[N];
	for (int i=0; i<N; i++) {x[i]=y[i]=z[i]=0;}

	float T=0;
	while ( fread(&T, 4, 1, fin)==1 )
	{
		if (T<t_min || T>t_max) { fseek(fin, N*4*(2+Cp+Fr+Nu), SEEK_CUR); continue; }
		for (int i=0; i<N; i++)
		{
			float buf[5];
			fread(&buf, 4, 2+Cp+Fr+Nu, fin);
			x[i] = buf[0];
			y[i] = buf[1];
			z[i] += buf[1+I];
		}
		count++;
	}
	cerr << count << endl;

	for (int i=0; i<N; i++)
	{
		printf("%g \t%g \t %g\n", x[i], y[i], z[i]);
	}

	return 0;
}
