#include <iostream>
#include <fstream>
#include <stdio.h>
#include <sstream>
#include <cstring>
#include <unistd.h>

#define dbgout cout
using namespace std;

int main(int argc, char **argv)
{
	if (argc < 2) {cerr << "Error! Please use: \ndata2gnumeric -f file [-t template] [-o output]" << endl; return -1; }
	FILE * fin;
	fstream fout;
	char temple[256]="", output[256]="";
	char exec[1024];
	char* files[128]; int filecount=0;
	int dbg=0; //debug flag
	FILE *pipe, *fin;
	double x, y, c;

	for (int i=0; i<128; i++) { files[i]=NULL; }

	opterr=0;
	int rez;
	while ( (rez = getopt(argc,argv,"df:ho:t:")) != -1)
	{
		switch (rez)
		{
			case 't': sscanf(optarg, "%s", temple); break;
			case 'f': files[filecount] = new char[256]; sscanf(optarg, "%s", files[filecount]); filecount++; break;
			case 'o': sscanf(optarg, "%s", output); break;
			case 'd': dbg=1; dbgout << "DEBUG mode ON\n\n"; break;
			case 'h': cerr << "Usage: \ndata2gnumeric -f file [-t template] [-o output]" << endl; return 0;
			case '?': printf("Unknown partameter '%s'. Ignoring\n", optarg); break;
		}
	}
	if (dbg)
	{
		dbgout << "reading " << filecount << " files:\n";
		for (int i=0; i<filecount; i++) { dbgout << "\t" << files[i] << endl; }
		
	}

	if (!filecount) { cerr << "No input files choosen. See 'data2gnumeric -h' for help." << endl; return -1; }
	if (!*output) 
	{
		if (dbg) {dbgout << "No output choosen. Using default: ";} 
		sprintf(output, "%s.gnumeric", files[0]); 
		if (dbg) {dbgout << output << endl << endl;}
	} else if (dbg)
	{ dbgout << "Using output: " << output << endl << endl; }
	
	
	int linesmax=0;
	if (dbg) {dbgout << "Counting max lines:\n";}
	for (int i=0; i<filecount; i++)
	{
		int lines;
		sprintf(exec, "wc -l %s", files[i]);
		pipe = popen(exec, "r");
		if (!pipe) continue;
		fscanf(pipe, "%d", &lines);
		if (lines > linesmax) linesmax = lines;
		if (dbg) {dbgout << "\t" << exec << " : " << lines << " ;\n";}
		pclose(pipe);
	}
	if (dbg) {dbgout << "Max=" << linesmax << endl;}

	sprintf(exec, "gzip > %s", output);
	pipe = popen(exec,"w");
	fprintf(pipe, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	fprintf(pipe, "<gnm:Workbook xmlns:gnm=\"http://www.gnumeric.org/v10.dtd\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.gnumeric.org/v9.xsd\">\n");
	fprintf(pipe, "<gnm:Version Epoch=\"1\" Major=\"9\" Minor=\"9\" Full=\"1.9.9\"/>\n");

	fprintf(pipe, "<gnm:SheetNameIndex>\n");
	for (int i=0; i<filescount; i++)
	{
		fprintf(pipe, "<gnm:SheetName gnm:Cols=\"256\" gnm:Rows=\"%d\">%s</gnm:SheetName>\n", linesmax, files[i]);
	}
	fprintf(pipe, "</gnm:SheetNameIndex>\n");

	fprintf(pipe, "<gnm:Sheets>\n");
	for (int i=0; i<filescount; i++)
	{
		fprintf(pipe, "<gnm:Sheet>\n");
		fprintf(pipe, "<gnm:Name>%s</gnm:Name>\n", files[i]);
		fprintf(pipe, "<gnm:MaxCol>-1</gnm:MaxCol>\n");
		fprintf(pipe, "<gnm:MaxRow>-1</gnm:MaxRow>\n");

		fprintf(pipe, "<gnm:Cells>\n");
		int col=0, row=0;
		char tmpline[255], value[32];
		fin = fopen(file[i], "r");
		while ( !feof(fin) )
		{
			fgets(tmpline, 255, fin);
			int ch=0;
			while(tmplime[ch]) if (tmpline[ch] == '.') { tmpline[ch]=','; ch++; }
			ch=0;
			while(tmpline[ch]) if (tmpline[ch] == ''
			sscanf(tmpline, "%s", value);
			VortexList->Copy(&Vort);
		}
		fclose(fin);
		fprintf(pipe, "</gnm:Cells>\n");
	}
	fprintf(pipe, "</gnm:Sheets>\n");


	fprintf(pipe, "<gnm:UIData SelectedTab=\"0\"/>\n");
	fprintf(pipe, "</gnm:Workbook>\n");
	
	pclose(pipe);
/*
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
*/
	return 0;
}
