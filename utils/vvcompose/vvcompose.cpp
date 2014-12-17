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

double parse(const char *arg, const char* fmt, const char* val)
{
	double res;
	int len;
	if (sscanf(val, fmt, &res, &len) != 1 || val[len] != '\0')
	{
		fprintf(stderr, "vvcompose: set: %s: bad value: %s\n", arg, val);
		exit(3);
	}
	return res;
}

void do_set(Space* S, const char *arg, const char *value)
{
	int len = 0;
	unsigned body_no;

	#define comp3d(vec) (*c=='x'?vec.x)
	#define M(fmt) (len=0, sscanf(arg, fmt, &len), arg[len] == '\0')
	#define ALERT() { fprintf(stderr, "vvcompose: set: ambiguous argument: %s\n", arg); exit(3); }
	#define parse_double(val) parse(arg, "%lg%n", val)
	#define parse_int(val)    parse(arg, "%d%n", val)
	#define strtotime(val) TTime::makeWithSecondsDecimal(parse_double(val))

	     if ( M("name%n") || M("caption%n") )       { S->caption = value; }
	else if ( M("t%n") || M("time%n") )             { S->Time = strtotime(value); }
	else if ( M("dt%n") )                           { S->dt = strtotime(value); }
	else if ( M("dts%n") || M("dt_save%n") )        { S->save_dt = strtotime(value); }
	else if ( M("dti%n") || M("dt_streak%n") )      { S->streak_dt = strtotime(value); }
	else if ( M("dtp%n") || M("dt_profile%n") )     { S->profile_dt = strtotime(value); }
	else if ( M("re%n") )                           { S->Re = parse_double(value); }
	else if ( M("pr%n") )                           { S->Pr = parse_double(value); }
	else if ( M("fin%n") || M("time_to_finish%n") ) { S->Finish = parse_double(value); }
	else if ( M("infx%n") || M("inf_speed.x%n") )   { S->InfSpeedX = value; }
	else if ( M("infy%n") || M("inf_speed.y%n") )   { S->InfSpeedY = value; }
	else if ( M("gravity.x%n") )                    { S->gravitation.x = parse_double(value); }
	else if ( M("gravity.y%n") )                    { S->gravitation.y = parse_double(value); }
	else if ( M("infg%n") || M("inf_circulation%n") ) { S->InfCirculation = parse_double(value); }
	else if ( sscanf(arg, "body[%u]%n", &body_no, &len) ||
	          sscanf(arg, "b%u%n", &body_no, &len) )
	{
		if (arg[len] != '.') ALERT();
		if (body_no >= S->BodyList->size_safe())
		{
			fprintf(stderr, "vvcompose: set: body[%u]: no such body\n", body_no);
			exit(3);
		}
		TBody *body = S->BodyList->at(body_no);
		arg += len;

		char c = 0;
		TVec3D *vec = NULL;
		     if ( len=0, sscanf(arg, "pos.%c%n",      &c, &len), !arg[len]) ) { vec = &body->pos; }
		else if ( len=0, sscanf(arg, "dpos.%c%n",     &c, &len), !arg[len]) ) { vec = &body->dPos; }
		else if ( len=0, sscanf(arg, "spring.%c%n",   &c, &len), !arg[len]) ) { vec = &body->k; }
		else if ( len=0, sscanf(arg, "damping.%c%n",  &c, &len), !arg[len]) ) { vec = &body->damping; }
		else if ( len=0, sscanf(arg, "speed.x%n",         &len), !arg[len]) ) { body->SpeedX = value; }
		else if ( len=0, sscanf(arg, "speed.y%n",         &len), !arg[len]) ) { body->SpeedY = value; }
		else if ( len=0, sscanf(arg, "speed.o%n",         &len), !arg[len]) ) { body->SpeedO = value; }
		else if ( len=0, sscanf(arg, "density%n",         &len), !arg[len]) ) { body->density = parse_double(value); }
		else if ( sscanf(arg, "root%n",            &len) == 1 && arg[len] == '\0')
			{ body->root_body = S->BodyList->at(parse_int(value)); }
		else ALERT();

		if (vec) switch (c)
		{
			case 'x': vec->r.x = parse_double(value); break;
			case 'y': vec->r.y = parse_double(value); break;
			case 'o': vec->o = parse_double(value); break;
			case 'd': vec->o = parse_double(value)*C_PI/180.0; break;
			default: ALERT();
		}
	}
	else ALERT();
	#undef M, ALERT
	#undef strtotime
	#undef parse_double, parse_int
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
