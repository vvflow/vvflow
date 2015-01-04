#include "core.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

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

template<typename T> T parse(const char *value);
template<> double parse(const char *val)
{
	double res;
	int len;
	if (sscanf(val, "%lg%n", &res, &len) != 1 || val[len])
	{
		fprintf(stderr, "vvcompose: set: bad double value: %s\n", val);
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
		fprintf(stderr, "vvcompose: set: bad int value: %s\n", val);
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

	#define comp3d(vec) (*c=='x'?vec.x)
	#define M(fmt) (len=0, sscanf(arg, fmt, &len), arg[len] == '\0')
	#define ALERT() { fprintf(stderr, "vvcompose: set: ambiguous argument: %s\n", arg_original); exit(3); }

	     if ( M("name%n") || M("caption%n") )       { S->caption = value; }
	else if ( M("t%n") || M("time%n") )             { S->Time = parse<TTime>(value); }
	else if ( M("dt%n") )                           { S->dt = parse<TTime>(value); }
	else if ( M("dts%n") || M("dt_save%n") )        { S->save_dt = parse<TTime>(value); }
	else if ( M("dti%n") || M("dt_streak%n") )      { S->streak_dt = parse<TTime>(value); }
	else if ( M("dtp%n") || M("dt_profile%n") )     { S->profile_dt = parse<TTime>(value); }
	else if ( M("re%n") )                           { S->Re = parse<double>(value); }
	else if ( M("pr%n") )                           { S->Pr = parse<double>(value); }
	else if ( M("fin%n") || M("time_to_finish%n") ) { S->Finish = parse<double>(value); }
	else if ( M("infx%n") || M("inf_speed.x%n") )   { S->InfSpeedX = value; }
	else if ( M("infy%n") || M("inf_speed.y%n") )   { S->InfSpeedY = value; }
	else if ( M("gravity.x%n") )                    { S->gravitation.x = parse<double>(value); }
	else if ( M("gravity.y%n") )                    { S->gravitation.y = parse<double>(value); }
	else if ( M("infg%n") || M("inf_circulation%n") ) { S->InfCirculation = parse<double>(value); }
	else if ( sscanf(arg, "body[%u]%n", &body_no, &len) ||
	          sscanf(arg, "b%u%n", &body_no, &len) )
	{
		if (arg[len] != '.') ALERT();
		if (body_no >= S->BodyList.size())
		{
			fprintf(stderr, "vvcompose: set: body[%u]: no such body\n", body_no);
			exit(3);
		}
		TBody *body = S->BodyList[body_no].get();
		arg += len+1;

		char c = 0;
		TVec3D *vec = NULL;
		TVec3D move_vec = TVec3D();
		     if ( (len=0, sscanf(arg, "pos.%c%n",      &c, &len), !arg[len]) ) { vec = &body->holder; }
		else if ( (len=0, sscanf(arg, "dpos.%c%n",     &c, &len), !arg[len]) ) { vec = &body->dpos; }
		else if ( (len=0, sscanf(arg, "move.%c%n",     &c, &len), !arg[len]) ) { vec = &move_vec; }
		else if ( (len=0, sscanf(arg, "spring.%c%n",   &c, &len), !arg[len]) ) { vec = &body->kspring; }
		else if ( (len=0, sscanf(arg, "damping.%c%n",  &c, &len), !arg[len]) ) { vec = &body->damping; }
		else if ( (len=0, sscanf(arg, "speed.x%n",         &len), !arg[len]) ) { body->speed_x = value; }
		else if ( (len=0, sscanf(arg, "speed.y%n",         &len), !arg[len]) ) { body->speed_y = value; }
		else if ( (len=0, sscanf(arg, "speed.o%n",         &len), !arg[len]) ) { body->speed_o = value; }
		else if ( (len=0, sscanf(arg, "density%n",         &len), !arg[len]) ) { body->density = parse<double>(value); }
		else if (  len=0, sscanf(arg, "root%n",            &len), !arg[len])
			{ body->root_body = S->BodyList[parse<int>(value)]; }
		else ALERT();

		if (vec) switch (c)
		{
			case 'x': vec->r.x = parse<double>(value); break;
			case 'y': vec->r.y = parse<double>(value); break;
			case 'o': vec->o = parse<double>(value); break;
			case 'd': vec->o = parse<double>(value)*C_PI/180.0; break;
			default: ALERT();
		}
		if (vec == &move_vec)
		{
			for (auto& latt: body->alist)
			{
				TVec dr = latt.corner - body->get_axis();
				latt.corner = move_vec.r + body->get_axis() + dr*cos(move_vec.o) + rotl(dr)*sin(move_vec.o);
			}
			body->doUpdateSegments();
			body->doFillProperties();
		}
	}
	else ALERT();
	#undef M, ALERT
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
	// else if ((sscanf(arg, "body[%u]%n", &body_no, &len) ||
	//           sscanf(arg, "b%u%n", &body_no, &len)) && !arg[len])
	// {
	// 	if (body_no >= S->BodyList->size_safe())
	// 	{
	// 		fprintf(stderr, "vvcompose: del: body[%u]: no such body\n", body_no);
	// 		exit(3);
	// 	}
	// 	deletion_list->push_back(S->BodyList->at(body_no));
	// }
	else
	{
		fprintf(stderr, "vvcompose: del: bad argument: %s\n", arg);
		exit(1);
	}
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
		else if (!strcmp(argv[i], "save")) { CHECK(1); S->Save(argv[i+1]); i+=2; }
		#undef CHECK
		else
		{
			fprintf(stderr, "vvcompose: bad command: %s\n", argv[i]);
			return 1;
		}
	}
	return 0;
}
