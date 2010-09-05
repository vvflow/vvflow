#include "libVVHD/core.h"

#include "libVVHD/convectivefast.h"
#include "libVVHD/diffmergefast.h"
#include "libVVHD/diffusivefast.h"
#include "libVVHD/utils.h"
#include "libVVHD/flowmove.h"

#include "iostream"
#include "fstream"
#include "malloc.h"
#include "string.h"
#include <signal.h>
#include <netcdfcpp.h>

#include "time.h"
using namespace std;

/******************************************************************************/

int quit;

void sig_handler(int sig)
{
	quit=1;
}

/******************************************************************************/

char *InfSpeedXsh;
char *InfSpeedYsh;
char *Rotationsh;

double InfSpeedX(double t) 
{
	if ((!InfSpeedXsh) || (!*InfSpeedXsh)) return 0;
	double result;
	char *exec; exec = (char*)(malloc(strlen(InfSpeedXsh)+32));
	sprintf(exec, "t=%lf; T=%lf; %s", t, t, InfSpeedXsh);

	FILE *pipe = popen(exec,"r");
	if (!pipe) return 0;
	if (!fscanf(pipe, "%lf", &result)) result=0;
	pclose(pipe);

	return result;
}

double InfSpeedY(double t)
{
	if ((!InfSpeedYsh) || (!*InfSpeedYsh)) return 0;
	double result;
	char *exec; exec = (char*)(malloc(strlen(InfSpeedYsh)+32));
	sprintf(exec, "t=%lf; T=%lf; %s", t, t, InfSpeedYsh);

	FILE *pipe = popen(exec,"r");
	if (!pipe) return 0;
	if (!fscanf(pipe, "%lf", &result)) result=0;
	pclose(pipe);

	return result;
}

double Rotation(double t) //функция вращения цилиндра
{
	if ((!Rotationsh) || (!*Rotationsh)) return 0;
	double result;
	char *exec; exec = (char*)(malloc(strlen(Rotationsh)+32));
	sprintf(exec, "t=%lf; T=%lf; %s", t, t, Rotationsh);

	FILE *pipe = popen(exec,"r");
	if (!pipe) return 0;
	if (!fscanf(pipe, "%lf", &result)) result=0;
	pclose(pipe);

	return result;
}

/******************************************************************************/

int main(int argc, char **argv)
{
	/* loading **************************/
	if (argc != 2) { cout << "Use: vvhdflow filename" << endl; return -1; }
	double Re;
	double dt;
	int BodyVorts;
	bool HeatEnabled;

	double TreeFarCriteria;
	double MinNodeSize;
	double MinG;
	double ConvEps;
	double MergeEps;

	int PrintFreq;

	NcFile dataFile(argv[1], NcFile::Write, 0, NcFile::Offset64Bits);
	if (!dataFile.is_valid())
	{
		cout << "Couldn't open file!\n";
		return -1;
	}

	NcAtt* att;
	
	att = dataFile.get_att("Re"); 				if (att) Re = att->as_double(0);
	att = dataFile.get_att("dt"); 				if (att) dt = att->as_double(0);
	att = dataFile.get_att("InfSpeedX"); 		if (att) InfSpeedXsh = att->as_string(0);
	att = dataFile.get_att("InfSpeedY"); 		if (att) InfSpeedYsh = att->as_string(0);
	att = dataFile.get_att("Rotation"); 		if (att) Rotationsh = att->as_string(0);
	att = dataFile.get_att("BodyVorts"); 		if (att) BodyVorts = att->as_int(0);
	att = dataFile.get_att("HeatEnabled"); 		if (att) HeatEnabled = att->as_ncbyte(0);
	att = dataFile.get_att("TreeFarCriteria"); 	if (att) TreeFarCriteria = att->as_double(0);
	att = dataFile.get_att("MinNodeSize"); 		if (att) MinNodeSize = att->as_double(0);
	att = dataFile.get_att("MinG"); 			if (att) MinG = att->as_double(0);
	att = dataFile.get_att("ConvEps"); 			if (att) ConvEps = att->as_double(0);
	att = dataFile.get_att("MergeEps"); 		if (att) MergeEps = att->as_double(0);
	att = dataFile.get_att("PrintFreq"); 		if (att) PrintFreq = att->as_int(0);

	cout << "LOAD SUCCESSFULL " << endl;
	cout << "\tRe = " << Re << endl;
	cout << "\tdt = " << dt << endl;
	cout << "\tBodyVorts = " << BodyVorts << endl;
	cout << "\tHeat enabled = " << (HeatEnabled?"YES":"NO") << endl << endl;

	cout << "\tInfSpeedX = " << InfSpeedXsh << endl;
	cout << "\tInfSpeedY = " << InfSpeedYsh << endl;
	cout << "\tRotation = " << Rotationsh << endl << endl;

	cout << "\tTreeFarCriteria = " << TreeFarCriteria << endl;
	cout << "\tMinNodeSize = " << MinNodeSize << endl;
	cout << "\tMinG = " << MinG << endl;
	cout << "\tConvEps = " << ConvEps << endl;
	cout << "\tMergeEps = " << MergeEps << endl;
	cout << "\tPrintFreq = " << PrintFreq << endl;

	/* initializing *********************/
	Space *S = new Space(1, 1, HeatEnabled, InfSpeedX, InfSpeedY, Rotation);
	S->ConstructCircle(BodyVorts);
	InitTree(S, TreeFarCriteria, MinNodeSize);
	InitFlowMove(S, dt, MinG);
	InitConvectiveFast(S, ConvEps);
	InitDiffMergeFast(S, Re, MergeEps*MergeEps);
	InitDiffusiveFast(S, Re);

	/* SIGINT part **********************/
	struct sigaction act;
	sigemptyset (&act.sa_mask);
	act.sa_handler = &sig_handler;
	act.sa_flags = 0;

	quit = 0;
	sigaction(SIGINT, &act, NULL);

	/* MAIN cycle ***********************/
	NcError err(NcError::silent_nonfatal);

	NcDim* ncTimeDim = dataFile.get_dim("Time");
	if (ncTimeDim)
	{
		//load
	} else
	{
		ncTimeDim = dataFile.add_dim("Time");
		if (!ncTimeDim) { cout << "can't add dimention" << endl; return -1; }
	}

	#define AddVar(VarName, VarCaption, type) \
		NcVar* VarName = dataFile.get_var(VarCaption);\
		if (!VarName) VarName = dataFile.add_var(VarCaption, type, ncTimeDim); \
		if (!VarName) return -1;

	AddVar(ncTimeVar, "Time", ncDouble);

	AddVar(ncXCoordVar, "XCoord", ncDouble);
	AddVar(ncXSpeedVar, "XSpeed", ncDouble);
	AddVar(ncYCoordVar, "YCoord", ncDouble);
	AddVar(ncYSpeedVar, "YSpeed", ncDouble);
	AddVar(ncAngleVar, "Angle", ncDouble);
	AddVar(ncRotSpeedVar, "RotSpeed", ncDouble);

	AddVar(ncVortCountVar, "VortCount", ncLong);
	AddVar(ncHeatCountVar, "HeatCount", ncLong);

	AddVar(ncForceXVar, "ForceX", ncDouble);
	AddVar(ncHDMXVar, "HDMX", ncDouble);
	AddVar(ncForceYVar, "ForceY", ncDouble);
	AddVar(ncHDMYVar, "HDMY", ncDouble);
	AddVar(ncCleanedVar, "Cleaned", ncInt);
	AddVar(ncMergedVar, "Merged", ncInt);

	#undef AddVar

	long t1 = clock();
	long N = ncTimeVar->num_vals();
	for (long i=N+1; (i<N+2000)&&(!quit); i++)
	{
		S->StartStep();
		BuildTree(1, 1, HeatEnabled);
		CalcConvectiveFast();
		CalcVortexDiffMergeFast();
//		CalcVortexDiffusiveFast();
		DestroyTree();

		MoveAndClean(true);

		BuildTree(1, 1, 0);
		CalcCirculationFast();
		DestroyTree();

		VortexShed();
		if (HeatEnabled) HeatShed();

		/* PRINT */

		if (!(i%PrintFreq)) 
		{
			fstream fout;
			char fname[64];
			sprintf(fname, "results/data%04ld.vort", i);
			fout.open(fname, ios::out);
			PrintVorticity(fout, S, false);
			fout.close();
			cout << "Printing \"" << fname << "\" finished. \n"; 
		}

		ncTimeVar->put_rec(&S->Time, i);
		ncXCoordVar->put_rec(&S->BodyX, i);
		ncYCoordVar->put_rec(&S->BodyY, i);
		ncAngleVar->put_rec(&S->Angle, i);

		double XSpeed = InfSpeedX(S->Time);
		double YSpeed = InfSpeedY(S->Time);
		double RotSpeed = Rotation(S->Time);
		ncXSpeedVar->put_rec(&XSpeed, i);
		ncYSpeedVar->put_rec(&YSpeed, i);
		ncRotSpeedVar->put_rec(&RotSpeed, i);

		ncVortCountVar->put_rec(&S->VortexList->size, i);
		//ncHeatCountVar->put_rec(&S->HeatList->size, i);

		double ForceX, ForceY;
		ForceX = S->ForceX/dt/PrintFreq; ForceY = S->ForceY/dt/PrintFreq;
		ncForceXVar->put_rec(&ForceX, i);
		ncForceYVar->put_rec(&ForceY, i);
		S->ForceX = S->ForceY = 0;

		double hdmx, hdmy;
		S->HydroDynamicMomentum(hdmx, hdmy);
		ncHDMXVar->put_rec(&hdmx, i);
		ncHDMYVar->put_rec(&hdmy, i);

		//ncCleanedVar->put_rec(, i);
		//ncMergedVar->put_rec(, i);
		S->FinishStep();
		cout << "step " << i << " finished. \t\t " << S->VortexList->size << " vortexes.\t\t " << DiffMergedFastV() << " merged\n";

	}
	cout << "TIME " << double(clock()-t1)/CLOCKS_PER_SEC << endl;

	/************************************/

	return 0;
}
