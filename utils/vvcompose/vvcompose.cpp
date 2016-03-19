#include "core.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include <algorithm>

void do_load(Space* S, const char *arg, const char *file)
{
	     if (!strcmp(arg, "hdf"))  { S->Load(file); }
	else if (!strcmp(arg, "body")) { S->LoadBody(file); }
	else if (!strcmp(arg, "vort")) { S->LoadVorticityFromFile(file); }
	else if (!strcmp(arg, "heat")) { S->LoadHeatFromFile(file); }
	else if (!strcmp(arg, "source")) { S->LoadSource(file); }
	else if (!strcmp(arg, "ink"))  { S->LoadStreak(file); }
	else if (!strcmp(arg, "ink_source")) { S->LoadStreakSource(file); }
	else
	{
		fprintf(stderr, "vvcompose ERROR: load: bad argument: %s\n", arg);
		exit(1);
	}
}

template<typename T> T parse(const char *value);
template<> double parse(const char *val)
{
	double res;
	int len;
	if (sscanf(val, "%lg%n", &res, &len) != 1 || val[len])
	{
		fprintf(stderr, "vvcompose ERROR: set: bad double value: %s\n", val);
		exit(3);
	}
	return res;
}
template<> int parse(const char *val)
{
	int res;
	int len;
	if (sscanf(val, "%d%n", &res, &len) != 1 || val[len])
	{
		fprintf(stderr, "vvcompose ERROR: set: bad int value: %s\n", val);
		exit(3);
	}
	return res;
}
template<> TTime parse(const char *val)
{
	return TTime::makeWithSecondsDecimal(parse<double>(val));
}

void do_set(Space* S, const char *arg, const char *value)
{
	int len = 0;
	unsigned body_no;
	const char* arg_original = arg;

	#define ALERT() { fprintf(stderr, "vvcompose ERROR: set: ambiguous argument: %s\n", arg_original); exit(3); }

	     if ( (len=0, sscanf(arg, "caption%n",         &len), !arg[len]) ) { S->caption = value; }
	else if ( (len=0, sscanf(arg, "time%n",            &len), !arg[len]) ) { S->Time = parse<TTime>(value); }
	else if ( (len=0, sscanf(arg, "dt%n",              &len), !arg[len]) ) { S->dt = parse<TTime>(value); }
	else if ( (len=0, sscanf(arg, "dt_save%n",         &len), !arg[len]) ) { S->dt_save = parse<TTime>(value); }
	else if ( (len=0, sscanf(arg, "dt_streak%n",       &len), !arg[len]) ) { S->dt_streak = parse<TTime>(value); }
	else if ( (len=0, sscanf(arg, "dt_profile%n",      &len), !arg[len]) ) { S->dt_profile = parse<TTime>(value); }
	else if ( (len=0, sscanf(arg, "re%n",              &len), !arg[len]) ) { S->Re = parse<double>(value); }
	else if ( (len=0, sscanf(arg, "pr%n",              &len), !arg[len]) ) { S->Pr = parse<double>(value); }
	else if ( (len=0, sscanf(arg, "inf_speed.x%n",     &len), !arg[len]) ) { S->InfSpeedX.setEvaluator(value); }
	else if ( (len=0, sscanf(arg, "inf_speed.y%n",     &len), !arg[len]) ) { S->InfSpeedY.setEvaluator(value); }
	else if ( (len=0, sscanf(arg, "inf_circulation%n", &len), !arg[len]) ) { S->InfCirculation = parse<double>(value); }
	else if ( (len=0, sscanf(arg, "gravity.x%n",       &len), !arg[len]) ) { S->gravitation.x = parse<double>(value); }
	else if ( (len=0, sscanf(arg, "gravity.y%n",       &len), !arg[len]) ) { S->gravitation.y = parse<double>(value); }
	else if ( (len=0, sscanf(arg, "time_to_finish%n",  &len), !arg[len]) ) { S->Finish = parse<double>(value); }
	else if ( (len=0, sscanf(arg, "body%02u%n", &body_no, &len)==1 && arg[len]=='.') )
	{
		if (body_no >= S->BodyList.size())
		{
			fprintf(stderr, "vvcompose ERROR: set: body%02u: no such body\n", body_no);
			exit(3);
		}
		TBody *body = S->BodyList[body_no].get();
		arg += len+1;

		char c = 0;
		TVec3D *vec = NULL;
		TVec3D move_vec = TVec3D();
		/**/ if ( (len=0, sscanf(arg, "holder_position.%c%n", &c, &len), !arg[len]) ) { vec = &body->holder; }
		else if ( (len=0, sscanf(arg, "delta_position.%c%n",  &c, &len), !arg[len]) ) { vec = &body->dpos; }
		else if ( (len=0, sscanf(arg, "collision_min.%c%n",   &c, &len), !arg[len]) ) { vec = &body->collision_min; }
		else if ( (len=0, sscanf(arg, "collision_max.%c%n",   &c, &len), !arg[len]) ) { vec = &body->collision_max; }
		else if ( (len=0, sscanf(arg, "label%n",                  &len), !arg[len]) ) { body->label = value; }
		else if ( (len=0, sscanf(arg, "move.%c%n",            &c, &len), !arg[len]) ) { vec = &move_vec; }
		else if ( (len=0, sscanf(arg, "spring_const.%c%n",    &c, &len), !arg[len]) ) { vec = &body->kspring; }
		else if ( (len=0, sscanf(arg, "spring_damping.%c%n",  &c, &len), !arg[len]) ) { vec = &body->damping; }
		else if ( (len=0, sscanf(arg, "speed.x%n",                &len), !arg[len]) ) { body->speed_x.setEvaluator(value); }
		else if ( (len=0, sscanf(arg, "speed.y%n",                &len), !arg[len]) ) { body->speed_y.setEvaluator(value); }
		else if ( (len=0, sscanf(arg, "speed.o%n",                &len), !arg[len]) ) { body->speed_o.setEvaluator(value); }
		else if ( (len=0, sscanf(arg, "density%n",                &len), !arg[len]) ) { body->density = parse<double>(value); }
		else if ( (len=0, sscanf(arg, "bounce%n",                 &len), !arg[len]) ) { body->bounce = parse<double>(value); }
		else if ( (len=0, sscanf(arg, "special_segment_no%n",     &len), !arg[len]) ) { body->special_segment_no = parse<int>(value); }
		else if ( (len=0, sscanf(arg, "boundary_condition%n",     &len), !arg[len]) )
		{
			/**/ if (!strcmp(value, "steady")) body->boundary_condition = bc_t::steady;
			else if (!strcmp(value, "kutta")) body->boundary_condition = bc_t::kutta;
			else
			{
				fprintf(stderr, "vvcompose ERROR: set: bad BC value: %s (valid: steady|kutta)\n", value);
				exit(3);
			}
		}
		else if ( (len=0, sscanf(arg, "heat_condition%n",         &len), !arg[len]) )
		{
			/**/ if (!strcmp(value, "neglect")) body->heat_condition = hc_t::neglect;
			else if (!strcmp(value, "isolate")) body->heat_condition = hc_t::isolate;
			else if (!strcmp(value, "const_t")) body->heat_condition = hc_t::const_t;
			else if (!strcmp(value, "const_w")) body->heat_condition = hc_t::const_w;
			else
			{
				fprintf(stderr, "vvcompose ERROR: set: bad HC value: %s (valid: neglect|isolate|const_t|const_w)\n", value);
				exit(3);
			}
		}
		else if ( (len=0, sscanf(arg, "general_slip%n",           &len), !arg[len]) )
		{
			int32_t general_slip = parse<int>(value);
			for (auto& latt: body->alist) latt.slip = general_slip;
		}
		else if ( (len=0, sscanf(arg, "heat_const%n",             &len), !arg[len]) )
		{
			double heat_const = parse<double>(value);
			for (auto& latt: body->alist) latt.heat_const = heat_const;
		}
		else if (  len=0, sscanf(arg, "root_body%n",              &len), !arg[len])
		{
			body->root_body = S->BodyList[parse<int>(value)];
		}
		else if (  len=0, sscanf(arg, "reverse%n",              &len), !arg[len])
		{
			std::reverse(body->alist.begin(),body->alist.end());
			body->doUpdateSegments();
    		body->doFillProperties();
		}
		else ALERT();

		if (vec) switch (c)
		{
			case 'x': vec->r.x = parse<double>(value); break;
			case 'y': vec->r.y = parse<double>(value); break;
			case 'o': vec->o = parse<double>(value); break;
			case 'd': vec->o = parse<double>(value)*C_PI/180.0; break;
			default: ALERT();
		}
		if (vec == &move_vec) body->move(move_vec, move_vec);
	}
	else ALERT();
	#undef ALERT
}

// when called with arg == "body[N]" the body is marked for deletion
// the actual deletion happens when you call do_del(S, NULL)
void do_del(Space *S, const char *arg)
{
	int len = 0;
	unsigned body_no = 0;
	// static auto deletion_list = new vector<TBody*>();
	// if (!arg)
	// {
	// 	// do deletion
	// }

	     if (!strcmp(arg, "vort")) { S->VortexList.clear(); }
	else if (!strcmp(arg, "heat")) { S->HeatList.clear(); }
	else if (!strcmp(arg, "ink"))  { S->StreakList.clear(); }
	else if (!strcmp(arg, "ink_source")) { S->StreakSourceList.clear(); }
	else if ( (len=0, sscanf(arg, "body%02u%n", &body_no, &len)==1 && !arg[len]) )
	{
		if (body_no >= S->BodyList.size())
		{
			fprintf(stderr, "vvcompose ERROR: del: no such body \"body%02u\"\n", body_no);
			exit(3);
		}
		S->BodyList.erase(S->BodyList.begin()+body_no);
	}
	else
	{
		fprintf(stderr, "vvcompose ERROR: del: bad argument: %s\n", arg);
		exit(1);
	}
}

int main(int argc, char *argv[])
{
	Space *S = new Space();
	int i = 1;
	if (argc < 2)
	{
		printf("usage: vvcompose COMMAND ARGS [...]\n");
		printf("\n");
		printf("COMMAND:\n");
		printf("load {hdf|body|vort|heat|ink|ink_source} FILE: load stuff from file \n");
		printf("save FILE: save space in hdf5 format\n");
		printf("del {bodyXX|vort|heat|ink|ink_source}: remove specific list from space\n");
		printf("set VARIABLE VALUE: set given parameter in space\n");
		printf("\n");
		printf("VARIABLE                         VALUE\n");
		printf("caption                          STR\n");
		printf("time                             DOUBLE\n");
		printf("dt                               DOUBLE\n");
		printf("dt_save                          DOUBLE\n");
		printf("dt_streak                        DOUBLE\n");
		printf("dt_profile                       DOUBLE\n");
		printf("re                               DOUBLE\n");
		printf("pr                               DOUBLE\n");
		printf("gravity.{x|y}                    DOUBLE\n");
		printf("inf_speed.{x|y}                  STR (math expression)\n");
		printf("inf_circulation                  DOUBLE\n");
		printf("time_to_finish                   DOUBLE\n");
		printf("bodyXX.speed.{x|y|o}             STR (math expression)\n");
		printf("bodyXX.root_body                 STR (in form: bodyXX)\n");
		printf("bodyXX.move.{x|y|o|d}            DOUBLE\n");
		printf("bodyXX.density                   DOUBLE\n");
		printf("bodyXX.holder_position.{x|y|o|d} DOUBLE\n");
		printf("bodyXX.delta_position.{x|y|o|d}  DOUBLE\n");
		printf("bodyXX.spring_const.{x|y|o}      DOUBLE\n");
		printf("bodyXX.spring_damping.{x|y|o}    DOUBLE\n");
		printf("bodyXX.general_slip              0|1\n");
		printf("bodyXX.special_segment_no        INT\n");
		printf("bodyXX.boundary_condition        steady|kutta\n");
		printf("bodyXX.heat_condition            neglect|isolate|const_t|const_w\n");
		printf("bodyXX.heat_const                DOUBLE\n");

		printf("bodyXX.bounce                    DOUBLE\n");
		printf("bodyXX.collision_min.{x|y|o|d}   DOUBLE\n");
		printf("bodyXX.collision_max.{x|y|o|d}   DOUBLE\n");
		return 0;
	}
	while (i<argc)
	{
		#define CHECK(x) if (i+x>=argc) {fprintf(stderr, "vvcompose ERROR: %s: not enought arguments\n", argv[i]); return 2; }
		     if (!strcmp(argv[i], "load")) { CHECK(2); do_load(S, argv[i+1], argv[i+2]); i+=3; }
		else if (!strcmp(argv[i], "set"))  { CHECK(2); do_set(S, argv[i+1], argv[i+2]); i+=3; }
		else if (!strcmp(argv[i], "del"))  { CHECK(1); do_del(S, argv[i+1]); i+=2; }
		else if (!strcmp(argv[i], "save"))
		{
			CHECK(1);

			#define DT_WARNING(STR) fprintf(stderr, "vvcompose WARNING: " STR " is not divisible by dt\n");
			if (!S->dt_save.divisibleBy(S->dt)) DT_WARNING("dt_save");
			if (!S->dt_streak.divisibleBy(S->dt)) DT_WARNING("dt_streak");
			if (!S->dt_profile.divisibleBy(S->dt)) DT_WARNING("dt_profile");
			#undef DT_WARNING

			S->Save(argv[i+1]);
			i+=2;
		}
		#undef CHECK
		else
		{
			fprintf(stderr, "vvcompose ERROR: bad command: %s\n", argv[i]);
			return 1;
		}
	}
	delete S;
	return 0;
}
