#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <math.h>

#include "core.h"

int main(int argc, char *argv[])
{
	if ( argc < 3)\
	{
		cerr << "Error! Please use: \vvcompose_exe -[bhov] file\n" <<
		        "-b{2,3,5} - body file with 2 3 or 5 columns\n" <<
		        "-h - heat file\n-o - output\n-v - vortexes\n";
		return -1;
	}

	Space *S = new Space();
	char *output;
	for (int i=1; i<argc; i+=2)
	{
		if (argv[i][0] != '-') { cerr << "Error\n"; return -1;}
		switch (argv[i][1])
		{
			case 'b': S->LoadBody(argv[i+1], argv[i][2]?int(argv[i][2])-'0':5); break;
			case 'v': S->LoadVorticityFromFile(argv[i+1]); break;
			case 'h': S->LoadHeatFromFile(argv[i+1]); break;
			case 'o': output = argv[i+1]; break;
			default: cerr << "Bad option: -" << argv[i][1] << endl; return -1;
		}
	}
	S->Save(output);
}
