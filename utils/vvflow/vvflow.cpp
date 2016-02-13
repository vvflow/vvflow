#include "iostream"
#include "fstream"
#include "stdio.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>

#include "core.h"
#include "epsfast.h"
#include "convectivefast.h"
#include "diffusivefast.h"
#include "flowmove.h"

#include "sensors.cpp"
#include "omp.h"
#define dbg(a) a
//#define dbg(a) cerr << "Doing " << #a << "... " << flush; a; cerr << "done\n";
#define seek(f) while (fgetc(f)!='\n'){}
using namespace std;

//#define OVERRIDEMOI 0.35

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		cerr << "Missing filename to load. You can create it with vvcompose utility.\n";
		return -1;
	}

	bool b_progress = false;
	if (!strcmp(argv[1], "--progress"))
	{
		b_progress = true;
		argv++;
	}

	Space *S = new Space();
	S->Load(argv[1]);

	// error checking
	#define RAISE(STR) { cerr << "vvflow ERROR: " << STR << endl; return -1; }
	if (S->Re == 0) RAISE("invalid value re=0");
	#undef RAISE

	char dir[256]; sprintf(dir, "results_%s", S->caption.c_str()); mkdir(dir, 0777);
	char profile[256]; sprintf(profile, "profile_%s", S->caption.c_str());
	char sensors_output[256]; sprintf(sensors_output, "velocity_%s", S->caption.c_str());


	Stepdata *stepdata = new Stepdata(S);
	stepdata->create("stepdata_%s.h5");

	// FILE *f = fopen(stepdata, "a");
	// #pragma omp parallel
	// #pragma omp master
	// 	fprintf(f, "\nOMP_NUM_THREADS = \t%d\n", omp_get_num_threads());
	// fprintf(f, "Working dir = \t%s\n", dir);
	// fprintf(f, "Git commit  = \t%s\n", S->getGitInfo());
	// fprintf(f, "Git diff    = \t%s\n", S->getGitDiff());
	// fflush(f);

	/**************************************************************************/

	// fprintf(f, "%-10s \t", "Time");
	// for (int i=0; i<S->BodyList->size(); i++)
	// fprintf(f, "Fx\t Fy\t Mz\t Friction_x\t Firc_y\t Fric_m\t NUsselt/L\t PosX\t PosY\t Angle\t DeltaPosX\t DeltaPosY\t DeltaAngle\t SpeedX\t SpeedY\t SpeedO\t ");
	// fprintf(f, "Nv\t Nh \n");

	double dl = S->AverageSegmentLength();
	double min_node_size = dl>0 ? dl*5 : 0;
	double max_node_size = dl>0 ? dl*100 : std::numeric_limits<double>::max();
	TSortedTree tr(S, 8, min_node_size, max_node_size);
	S->Tree = &tr;
	convectivefast conv(S);
	epsfast eps(S);
	diffusivefast diff(S);
	flowmove fm(S);
	sensors sens(S, &conv, (argc>2)?argv[2]:NULL, sensors_output);
	#ifdef OVERRIDEMOI
		S->BodyList->at(0)->overrideMoi_c(OVERRIDEMOI);
	#endif

	while (S->Time < S->Finish + S->dt/2)
	{
		if (S->BodyList.size())
		{
			dbg(tr.build());
			for (int iter=1;; iter++)
			{
				// решаем СЛАУ
				conv.CalcCirculationFast();
				if (!fm.DetectCollision(iter))
					break;
				else if (iter>2)
				{
					fprintf(stderr, "Collition is not resolved in 2 iterations\n");
					break;
				}
			}
			dbg(tr.destroy());
		}

		dbg(fm.HeatShed());

		if (S->Time.divisibleBy(S->dt_save)  && (double(S->Time) > 0))
		{
			char tmp_filename[256]; sprintf(tmp_filename, "%s/%%06d.h5", dir);
			S->Save(tmp_filename);
		}

		dbg(fm.VortexShed());
		dbg(fm.StreakShed());

		dbg(S->CalcForces());
		if (S->Time > 0) S->SaveProfile(profile, val::Cp | val::Fr | val::Nu);
		stepdata->write();
		/*fprintf(f, "%-10g \t", double(S->Time));
		const_for(S->BodyList, llbody)
		{
		fprintf(f, "%-+20e\t %-+20e\t %-+20e\t %-+20e\t %-+20e\t %-+20e\t %-+20e\t ",
		             (**llbody).Force_export.r.x,
		             (**llbody).Force_export.r.y,
		             (**llbody).Force_export.o,
		             (**llbody).Friction.r.x,
		             (**llbody).Friction.r.y,
		             (**llbody).Friction.o,
		             (**llbody).Nusselt);
		fprintf(f, "%-+20e\t %-+20e\t %-+20e\t %-+20e\t %-+20e\t %-+20e\t ",
		             (**llbody).pos.r.x,
		             (**llbody).pos.r.y,
		             (**llbody).pos.o,
		             (**llbody).dPos.r.x,
		             (**llbody).dPos.r.y,
		             (**llbody).dPos.o);
		fprintf(f, "%-+20e\t %-+20e\t %-+20e\t ",
		             (**llbody).Speed_slae.r.x,
		             (**llbody).Speed_slae.r.y,
		             (**llbody).Speed_slae.o);
		}
		fprintf(f, "%-10ld \t%-10ld \n",
		             S->VortexList->size_safe(),
		             S->HeatList->size_safe());
		fflush(f);*/

		S->ZeroForces();

		dbg(tr.build());
		dbg(eps.CalcEpsilonFast(true));
		dbg(conv.CalcBoundaryConvective());
		dbg(conv.CalcConvectiveFast());
		dbg(sens.output());
		dbg(diff.CalcVortexDiffusiveFast());
		dbg(diff.CalcHeatDiffusiveFast());
		dbg(tr.destroy());

		dbg(fm.MoveAndClean(true));
	        #ifdef OVERRIDEMOI
        	        S->BodyList->at(0)->overrideMoi_c(OVERRIDEMOI);
	        #endif
		dbg(fm.CropHeat());
		S->Time = TTime::add(S->Time, S->dt);

		if (b_progress)
			fprintf(stderr, "\r%-10g \t%-10zd \t%-10zd", double(S->Time), S->VortexList.size(), S->HeatList.size());
	}

	if (b_progress)
		fprintf(stderr, "\n");
	// fclose(f);
	stepdata->close();
}
