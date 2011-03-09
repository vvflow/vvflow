#include "body.h"
#include "stdio.h"
#include "iostream"
using namespace std;

TBody::TBody()
{
	List = new TList<TObject>();
}

int TBody::LoadFromFile(const char* filename)
{
	if ( !List ) return -1;
	FILE *fin;

	fin = fopen(filename, "r");
	if (!fin) { cerr << "No file called " << filename << endl; return -1; } 

	TObject Obj; ZeroObject(Obj);
	char line[255];
	while ( fgets(line, 254, fin) )
	{
		if (sscanf(line, "%lf\t%lf\n", &Obj.rx, &Obj.ry) == 2)
			List->Copy(&Obj);
	}
	fclose(fin);

	return 0;
}

bool TBody::ObjectIsValid(TObject &obj)
{
	return ( (obj.x*obj.x + obj.y*obj.y)>1 );
}
