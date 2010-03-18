#include <iostream>
#include <fstream>
#include <stdio.h>
#include <sstream>
#include <cstring>

using namespace std;

int main(int argc, char **argv)
{
	if (!strcmp(argv[1], "--help")) {cerr << "Syntax: data2gnumeric [files] BigColCount SmallColCount" << endl; return -1; }
	FILE * fin;
	fstream fout;
	char str[256];
	int i=0, bigcols; sscanf(argv[argc-2], "%d", &bigcols);
	int smallcols; sscanf(argv[argc-1], "%d", &smallcols);
	double x, y, c;

	sprintf(str, "data.gnumeric", argv[1]);
	fout.open(str, ios::out);
	fout << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2//EN\">\n" <<
			"<html>\n<head>\n\t<title>Tables</title>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n" << 
			"\t<!-- \"G_PLUGIN_FOR_HTML\" -->\n<style type=\"text/css\">\ntt {\n\tfont-family: courier;\n}\ntd {\n" <<
	"\tfont-family: helvetica, sans-serif;\n}\ncaption {\n\tfont-family: helvetica, sans-serif;\n\tfont-size: 14pt;\n" <<
	"\ttext-align: left;\n}\n</style>\n</head>\n<body>\n";
	locale us("");
	fout.imbue( us );
	for (int j=1; j<argc-2; j++)
	{
		fout << "<p><table border=\"1\" cellpadding=\"3\" cellspacing=\"0\">\n<caption>" << argv[j] << "</caption>" << endl;
		//cerr << n << endl;
		fout << "<tr>\n";
		
		fin = fopen(argv[j],"r");
		i=0;
		do
		{
			i++;
			if (i==(bigcols*smallcols+1)) { fout << "</tr>\n<tr>\n"; i=1; }
			fscanf(fin, "%lf", &x);
			fout << "\t<td>" << x << "</td>";
			if (i%smallcols==0) { fout << "\t<td></td>\n"; }
		} while (!feof(fin));
		fout << "\n</tr>\n</table>" << endl;
		
		fclose(fin);
	}
	fout << "</body>\n</html>" << endl;

	fout.close();

	return 0;
}
