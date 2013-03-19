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

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		fprintf(stdout, "vvcompose -i -b235 -bvx -bvy -bvo -bx -by -ba -bdx -bdy -bda -bkx -bky -bka -brho -mx -my -ma -md -v -h -s -ss -ix -iy -ig -gx -gy -t -dt -dt_save -dt_streak -dt_profile -re -pr -finish -name -o -remove\n");
		fprintf(stderr, "Options:\n-b{2,3,5} filename --- body file with 2 3 or 5 columns\n");
		fprintf(stderr, "-bvx, -bvy, -bvo './script.sh $t' --- script for body motion speed (vx, vy, omega)\n");
		fprintf(stderr, "-bx, -by, -ba float --- body initial coordinates (x, y, alpha)\n");
		fprintf(stderr, "-bdx, -bdy, -bda float --- body spring deformation (delta x, delta y, delta alpha)\n");
		fprintf(stderr, "-bkx, -bky, -bka float --- body spring koefficient\n");
		fprintf(stderr, "-brho float --- body density\n");
		fprintf(stderr, "-mx, -my float --- move body without changing position and deltaposition variables\n");
		fprintf(stderr, "-ma, -md float --- rotate body around r_0. Value is in radians and degrees correspondigly\n");
		fprintf(stderr, "Also you can put number after -b?? option (like -bvx2 'echo 0'). Numbers start with 1.");
		fprintf(stderr, " It will change corresponding parameter of selected body\n");

		fprintf(stderr, "-v, -h, -s, -ss filename --- files of vortexes, heat particles, streaks, streak sources (need 3 columns)\n");
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

	Space *S = new Space();
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
			getBodyFromArg(S, argv[i]+4)->SpeedX->initWithString(argv[i+1]);
		} else if (beginsWith(argv[i], "-bvy"))
		{
			getBodyFromArg(S, argv[i]+4)->SpeedY->initWithString(argv[i+1]);
		} else if (beginsWith(argv[i], "-bvo"))
		{
			getBodyFromArg(S, argv[i]+4)->SpeedO->initWithString(argv[i+1]);
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
		} else if (beginsWith(argv[i], "-brho"))
		{
			getBodyFromArg(S, argv[i]+5)->density = atof(argv[i+1]);


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
			S->InfSpeedX->initWithString(argv[i+1]);
		} else if (!strcmp(argv[i], "-iy"))
		{
			S->InfSpeedY->initWithString(argv[i+1]);
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
			S->name = argv[i+1];
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
