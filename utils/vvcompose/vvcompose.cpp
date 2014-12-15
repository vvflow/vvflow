#include "core.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

bool beginsWith(const char* string, const char* begining)
{
	for (int i=0; begining[i]; i++)
	{
		if (string[i] != begining[i]) return false;
	}

	return true;
}

TBody* getBodyFromArg(Space* S, const char* arg)
{
	if (*arg)
		return S->BodyList->at(atoi(arg)-1);
	else
		return *(S->BodyList->end()-1);
}

void transformBody(TBody* body, double x, double y, double a)
{
	const_for (body->List, lobj)
	{
		TVec dr = lobj->corner - (body->pos.r + body->dPos.r);
		lobj->corner = TVec(x, y) + body->pos.r + body->dPos.r + dr*cos(a) + rotl(dr)*sin(a);
	}
	body->doUpdateSegments();
	body->doFillProperties();
}

void do_load(Space* S, const char *arg, const char *file)
{
	     if (!strcmp(arg, "hdf"))  { S->Load(file); }
	else if (!strcmp(arg, "body")) { S->LoadBody(file); }
	else if (!strcmp(arg, "vort")) { S->LoadVorticityFromFile(file); }
	else if (!strcmp(arg, "heat")) { S->LoadHeatFromFile(file); }
	else if (!strcmp(arg, "ink"))  { S->LoadStreak(file); }
	else if (!strcmp(arg, "ink_source")) { S->LoadStreakSource(file); }
	else
	{
		fprintf(stderr, "vvcompose: load: bad argument: %s\n", arg);
		exit(1);
	}
}

void do_set(Space* S, const char *arg, const char *value)
{
	unsigned body_number;
	char argv[4][32];
	int argc = sscanf(arg, "%[^.].%[^.].%[^.].%[^.]", argv[0], argv[1], argv[2], argv[3]);

	#define CHECK(x) if (argc > x) \
		{ \
			fprintf(stderr, "vvcompose: set: ambiguous argument: %s", argv[0]); \
			for (int i=1; i<argc; i++) \
				fprintf(stderr, ".%s", argv[i]); \
			fprintf(stderr, "\n"); \
			exit(3); \
		}
	     if (!strcmp(argv[0], "name")            || !strcmp(argv[0], "caption")) { CHECK(1); }
	else if (!strcmp(argv[0], "t")               || !strcmp(argv[0], "time")) {}
	else if (!strcmp(argv[0], "dt")              || !strcmp(argv[0], "dt")) {}
	else if (!strcmp(argv[0], "dts")             || !strcmp(argv[0], "dt_save")) {}
	else if (!strcmp(argv[0], "dti")             || !strcmp(argv[0], "dt_streak")) {}
	else if (!strcmp(argv[0], "dtp")             || !strcmp(argv[0], "dt_profile")) {}
	else if (!strcmp(argv[0], "re")              || !strcmp(argv[0], "re")) {}
	else if (!strcmp(argv[0], "pr")              || !strcmp(argv[0], "pr")) {}
	else if (!strcmp(argv[0], "inf_marker")      || !strcmp(argv[0], "inf_marker")) {}
	else if (!strcmp(argv[0], "inf")             || !strcmp(argv[0], "inf_speed")) {}
	else if (!strcmp(argv[0], "inf_circulation") || !strcmp(argv[0], "inf_circulation")) {}
	else if (!strcmp(argv[0], "g")               || !strcmp(argv[0], "gravity")) {}
	else if (!strcmp(argv[0], "fin")             || !strcmp(argv[0], "time_to_finish")) {}
	else if (sscanf(arg, "body[%u].", &body_number) == 1 ||
		     sscanf(arg, "b%u.", &body_number) == 1)
	{

	}
	#undef CHECK
	else
	{
		fprintf(stderr, "vvcompose: set: bad argument: %s\n", argv[0]);
		exit(1);
	}
	return;

	// if (sscanf(arg, "blist[%u].", &body_number) == 1)
	// {
	// 	// TBody *body = S->BodyList[body_number];
	// 	if (sscanf(arg, "%*[^.].%31[^.].%1[xyo]", tmp[0], tmp[1]) == 2)
	// 	{
	// 		printf("set blist: %d %s %s\n", body_number, tmp[0], tmp[1]);
	// 	}
	// }
	// if (sscanf(arg, "infspeed.%1[xy]", tmp[0]) == 1)
	// {
	// 	printf("set infspeed: %s\n", tmp[0]);
	// }
	// else
	// {
	// 	sscanf(arg, "%31[^.]", tmp[0]);
	// 	fprintf(stderr, "vvcompose: set: bad argument: %s\n", tmp[0]);
	// }
}

void do_del(Space *S, const char *arg)
{

}

int main(int argc, char *argv[])
{
	Space *S = new Space();
	int i = 1;
	while (i<argc)
	{
		#define CHECK(x) if (i+x>=argc) {fprintf(stderr, "vvcompose: %s: not enought arguments\n", argv[i]); return 2; }
		     if (!strcmp(argv[i], "load")) { CHECK(2); do_load(S, argv[i+1], argv[i+2]); i+=3; }
		else if (!strcmp(argv[i], "set"))  { CHECK(2); do_set(S, argv[i+1], argv[i+2]); i+=3; }
		else if (!strcmp(argv[i], "del"))  { CHECK(1); do_del(S, argv[i+1]); i+=2; }
		else if (!strcmp(argv[i], "save")) { CHECK(1); S->Save(argv[i+1]); i+=3; }
		#undef CHECK
		else
		{
			fprintf(stderr, "vvcompose: bad command: %s\n", argv[i]);
			return 1;
		}
	}
	return 0;

	if (argc < 3)
	{
		fprintf(stdout, "vvcompose -i -b235 -bvx -bvy -bvo -bx -by -ba -bdx -bdy -bda -bkx -bky -bka -brho -mx -my -ma -md -v -h -s -ss -ix -iy -ig -gx -gy -t -dt -dt_save -dt_streak -dt_profile -re -pr -finish -name -o -remove\n");
		fprintf(stderr, "Options:\n-b{2,3,5} filename --- body file with 2 3 or 5 columns\n");
		fprintf(stderr, "-bvx, -bvy, -bvo './script.sh $t' --- script for body motion speed (vx, vy, omega)\n");
		fprintf(stderr, "-bx, -by, -ba float --- body initial coordinates (x, y, alpha)\n");
		fprintf(stderr, "-bdx, -bdy, -bda float --- body spring deformation (delta x, delta y, delta alpha)\n");
		fprintf(stderr, "-bkx, -bky, -bka float --- body spring koefficient\n");
		fprintf(stderr, "-bkdx, -bkdy, -bkda float --- body spring damping\n");
		fprintf(stderr, "-brho float --- body density\n");
		fprintf(stderr, "-broot int --- body root\n");
		fprintf(stderr, "-mx, -my float --- move body without changing position and deltaposition variables\n");
		fprintf(stderr, "-ma, -md float --- rotate body around r_0. Value is in radians and degrees correspondigly\n");
		fprintf(stderr, "Also you can put number after -b?? option (like -bvx2 'echo 0'). Numbers start with 1.");
		fprintf(stderr, " It will change corresponding parameter of selected body\n");

		// fprintf(stderr, "-v, -h, -s, -ss filename --- files of vortexes, heat particles, streaks, streak sources (need 3 columns)\n");
		fprintf(stderr, "-ix, -iy './script.sh $t' --- script for inf speed\n");
		fprintf(stderr, "-ig float --- constant circulation at infinity\n");
		fprintf(stderr, "-gx, -gy float --- gravitation\n");
		fprintf(stderr, "-t float --- if you want to override time\n");
		fprintf(stderr, "-dt, -dt_save, -dt_streak, -dt_profile float ---different dts\n");
		fprintf(stderr, "-re, -pr float --- 1/nyu and pr numbers\n");
		fprintf(stderr, "-name 'name' --- used in dir names. defaults to output filename\n");
		fprintf(stderr, "-finish float --- time to stop computation. defaults to DBL_MAX\n");
		fprintf(stderr, "-i, -o filename --- input and output files. Input arg should be mentioned first.\n");
		fprintf(stderr, "-remove b|v|h|s|ss --- remove selected list\n");
		fprintf(stderr, "First line of this text can be concatenated to your script.\n");
		return -1;
	}

	// Space *S = new Space();
	char *output;
	for (int i=1; i<argc; i+=2)
	{
		if (!strcmp(argv[i], "-i"))
		{
			S->Load(argv[i+1]);
		} else if (!strcmp(argv[i], "-b") || !strcmp(argv[i], "-b2") || !strcmp(argv[i], "-b3") || !strcmp(argv[i], "-b5"))
		{
			S->LoadBody(argv[i+1], argv[i][2]?atoi(argv[i]+2):5);
		} else if (beginsWith(argv[i], "-bvx"))
		{
			getBodyFromArg(S, argv[i]+4)->SpeedX = argv[i+1];
		} else if (beginsWith(argv[i], "-bvy"))
		{
			getBodyFromArg(S, argv[i]+4)->SpeedY = argv[i+1];
		} else if (beginsWith(argv[i], "-bvo"))
		{
			getBodyFromArg(S, argv[i]+4)->SpeedO = argv[i+1];
		} else if (beginsWith(argv[i], "-bx"))
		{
			getBodyFromArg(S, argv[i]+3)->pos.r.x = atof(argv[i+1]);
		} else if (beginsWith(argv[i], "-by"))
		{
			getBodyFromArg(S, argv[i]+3)->pos.r.y = atof(argv[i+1]);
		} else if (beginsWith(argv[i], "-ba"))
		{
			getBodyFromArg(S, argv[i]+3)->pos.o = atof(argv[i+1]);
		} else if (beginsWith(argv[i], "-bdx"))
		{
			getBodyFromArg(S, argv[i]+4)->dPos.r.x = atof(argv[i+1]);
		} else if (beginsWith(argv[i], "-bdy"))
		{
			getBodyFromArg(S, argv[i]+4)->dPos.r.y = atof(argv[i+1]);
		} else if (beginsWith(argv[i], "-bda"))
		{
			getBodyFromArg(S, argv[i]+4)->dPos.o = atof(argv[i+1]);
		} else if (beginsWith(argv[i], "-bkx"))
		{
			getBodyFromArg(S, argv[i]+4)->k.r.x = atof(argv[i+1]);
		} else if (beginsWith(argv[i], "-bky"))
		{
			getBodyFromArg(S, argv[i]+4)->k.r.y = atof(argv[i+1]);
		} else if (beginsWith(argv[i], "-bka"))
		{
			getBodyFromArg(S, argv[i]+4)->k.o = atof(argv[i+1]);
		} else if (beginsWith(argv[i], "-bkdx"))
		{
			getBodyFromArg(S, argv[i]+5)->damping.r.x = atof(argv[i+1]);
		} else if (beginsWith(argv[i], "-bkdy"))
		{
			getBodyFromArg(S, argv[i]+5)->damping.r.y = atof(argv[i+1]);
		} else if (beginsWith(argv[i], "-bkda"))
		{
			getBodyFromArg(S, argv[i]+5)->damping.o = atof(argv[i+1]);
		} else if (beginsWith(argv[i], "-brho"))
		{
			getBodyFromArg(S, argv[i]+5)->density = atof(argv[i+1]);
		} else if (beginsWith(argv[i], "-broot"))
		{
			getBodyFromArg(S, argv[i]+6)->root_body = S->BodyList->at(atof(argv[i+1])-1);


		} else if (beginsWith(argv[i], "-mx"))
		{
			transformBody(getBodyFromArg(S, argv[i]+3), atof(argv[i+1]), 0, 0);
		} else if (beginsWith(argv[i], "-my"))
		{
			transformBody(getBodyFromArg(S, argv[i]+3), 0, atof(argv[i+1]), 0);
		} else if (beginsWith(argv[i], "-ma"))
		{
			transformBody(getBodyFromArg(S, argv[i]+3), 0, 0, atof(argv[i+1]));
		} else if (beginsWith(argv[i], "-md"))
		{
			transformBody(getBodyFromArg(S, argv[i]+3), 0, 0, atof(argv[i+1])*C_PI/180.);


		} else if (!strcmp(argv[i], "-v"))
		{
			S->LoadVorticityFromFile(argv[i+1]);
		} else if (!strcmp(argv[i], "-h"))
		{
			S->LoadHeatFromFile(argv[i+1]);
		} else if (!strcmp(argv[i], "-s"))
		{
			S->LoadStreak(argv[i+1]);
		} else if (!strcmp(argv[i], "-ss"))
		{
			S->LoadStreakSource(argv[i+1]);
		} else if (!strcmp(argv[i], "-ix"))
		{
			S->InfSpeedX = argv[i+1];
		} else if (!strcmp(argv[i], "-iy"))
		{
			S->InfSpeedY = argv[i+1];
		} else if (!strcmp(argv[i], "-ig"))
		{
			S->InfCirculation = atof(argv[i+1]);
		} else if (!strcmp(argv[i], "-gx"))
		{
			S->gravitation.x = atof(argv[i+1]);
		} else if (!strcmp(argv[i], "-gy"))
		{
			S->gravitation.y = atof(argv[i+1]);
		} else if (!strcmp(argv[i], "-t"))
		{
			S->Time = TTime::makeWithSecondsDecimal(atof(argv[i+1]));
		} else if (!strcmp(argv[i], "-dt"))
		{
			S->dt = TTime::makeWithSecondsDecimal(atof(argv[i+1]));
		} else if (!strcmp(argv[i], "-dt_save"))
		{
			S->save_dt = TTime::makeWithSecondsDecimal(atof(argv[i+1]));
		} else if (!strcmp(argv[i], "-dt_streak"))
		{
			S->streak_dt = TTime::makeWithSecondsDecimal(atof(argv[i+1]));
		} else if (!strcmp(argv[i], "-dt_profile"))
		{
			S->profile_dt = TTime::makeWithSecondsDecimal(atof(argv[i+1]));
		} else if (!strcmp(argv[i], "-re"))
		{
			S->Re = atof(argv[i+1]);
		} else if (!strcmp(argv[i], "-pr"))
		{
			S->Pr = atof(argv[i+1]);
		} else if (!strcmp(argv[i], "-finish"))
		{
			S->Finish = atof(argv[i+1]);
		} else if (!strcmp(argv[i], "-name"))
		{
			S->caption = argv[i+1];
		} else if (!strcmp(argv[i], "-o"))
		{
			output = argv[i+1];
		} else if (!strcmp(argv[i], "-remove"))
		{
			if (!strcmp(argv[i+1], "b"))
				S->BodyList->clear();
			else if (!strcmp(argv[i+1], "v"))
				S->VortexList->clear();
			else if (!strcmp(argv[i+1], "h"))
				S->HeatList->clear();
			else if (!strcmp(argv[i+1], "s"))
				S->StreakList->clear();
			else if (!strcmp(argv[i+1], "ss"))
				S->StreakSourceList->clear();
			else fprintf(stderr, "Unknown remove option %s\n", argv[i+1]);
		} else
		{
			fprintf(stderr, "Unknown option %s\n", argv[i]);
			return -1;
		}
	}
	S->Save(output);

	return 0;
}
