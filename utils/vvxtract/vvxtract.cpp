#include "core.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

// void attribute_print(const char *name, const char *str)
// {
// 	if (!strlen(str)) return;
// 	printf("%-15s \"%s\"\n", name, str);
// }

void attribute_print(const char *name, std::string str)
{
	if (str.empty()) return;
	printf("%-15s \"%s\"\n", name, str.c_str());
}

void attribute_print(const char *name, TTime time)
{
	if (time.value == INT32_MAX) return;
	printf("%-15s %lg\n", name, double(time));
}

void attribute_print(const char *name, bool value)
{
	printf("%-15s %s\n", name, value?"True":"False");
}

void attribute_print(const char *name, double value, double ignored=0);
void attribute_print(const char *name, double value, double ignored)
{
	if (value == ignored) return;
	printf("%-15s %lg\n", name, value);
}

void attribute_print(const char *name, long int value)
{
	if (value == 0) return;
	printf("%-15s %ld\n", name, value);
}

void attribute_print(const char *name, TVec vec)
{
	if (vec.iszero()) return;
	printf("%-15s (%lg, %lg)\n", name, vec.x, vec.y);
}

void attribute_print(const char *name, TVec3D vec3d, double ignored=0);
void attribute_print(const char *name, TVec3D vec3d, double ignored)
{
	if (vec3d.r.x == ignored && vec3d.r.y == ignored && vec3d.o == ignored) return;
	printf("%-15s (%lg, %lg, %lg)\n", name, vec3d.r.x, vec3d.r.y, vec3d.o);
}

void print_info(Space *S)
{
	attribute_print("Caption:", S->caption);
	attribute_print("Time:", S->Time);
	attribute_print("dt:", S->dt);
	attribute_print("Save dt:", S->save_dt);
	attribute_print("Streak dt:", S->streak_dt);
	attribute_print("Profile dt:", S->profile_dt);
	attribute_print("1/nyu:", S->Re);
	attribute_print("Pr:", S->Pr);
	attribute_print("Inf marker:", S->InfMarker);
	attribute_print("Inf speed_x:", S->InfSpeedX);
	attribute_print("Inf speed_y:", S->InfSpeedY);
	attribute_print("Inf gamma:", S->InfCirculation);
	attribute_print("Gravity:", S->gravitation);
	attribute_print("Finish:", S->Finish);
}

void print_body(TBody *body)
{
	attribute_print("holder_position", body->holder, INT32_MAX);
	attribute_print("delta_position", body->dpos);
	attribute_print("speed_x", body->speed_x);
	attribute_print("speed_y", body->speed_y);
	attribute_print("speed_o", body->speed_o);
	attribute_print("speed_slae", body->speed_slae);
	attribute_print("speed_slae_prev", body->speed_slae_prev);
	attribute_print("spring_const", body->kspring, -1);
	attribute_print("spring_damping", body->damping);
	attribute_print("density", body->density, 1);
	attribute_print("force_hydro", body->force_hydro);
	attribute_print("force_holder", body->force_holder);
	attribute_print("friction_prev", body->friction_prev);
}

int main(int argc, char **argv)
{
	if (argc<2)
	{
		fprintf(stdout, "%s filename.h5\n", argv[0]);
		return 1;
	}

	std::string info[3];
	Space *S = new Space();
	S->Load(argv[1], info);

	printf("General:\n");
	print_info(S);
	attribute_print("VortexList:", long(S->VortexList.size()));
	attribute_print("BodyList:", long(S->BodyList.size()));
	attribute_print("HeatList:", long(S->HeatList.size()));
	attribute_print("InkList:", long(S->StreakList.size()));
	attribute_print("InkSourceList:", long(S->StreakSourceList.size()));
	
	for (auto& lbody: S->BodyList)
	{
		printf("\n%s:\n", lbody->get_name().c_str());
		print_body(lbody.get());
	}

	printf("\nNotes:\n");
	attribute_print("Local time:", info[2].c_str());
	attribute_print("Git info:", info[0].c_str());
	attribute_print("Git diff:", info[1].c_str());
}