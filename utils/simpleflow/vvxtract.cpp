#include "core.h"
#include "stdio.h"
#include "string.h"
#include "time.h"

int main(int argc, char **argv)
{
	if (argc<2) {fprintf(stderr, "Usage: vvxtract file.vb [bookmarks]\n"); return 0;}
	Space *S = new Space();
	S->Load(argv[1]);

	if (argc < 3)
	{
			printf("Name  = %s\n", S->name);
			printf("Date  = %s", asctime(localtime(&S->realtime)));
			printf("Time  = %g\n", double(S->Time));
			printf("dt    = %g\n", double(S->dt));
			printf("Re    = %g\n", S->Re);
		if (S->Pr)
			printf("Pr    = %g\n", S->Pr);

		if (strlen(S->InfSpeedX->getScript()))
			printf("InfSpeedX = %s\n", S->InfSpeedX->getScript());
		if (strlen(S->InfSpeedY->getScript()))
			printf("InfSpeedY = %s\n", S->InfSpeedY->getScript());
			printf("InfSpeed  = (%g, %g)\n", S->InfSpeed().x, S->InfSpeed().y);
			printf("InfMarker = (%g, %g)\n", S->InfMarker.x, S->InfMarker.y);
		if (S->InfCirculation)
			printf("InfGamma  = %g\n", S->InfCirculation);
		if (!S->gravitation.iszero())
			printf("gravitation = (%g, %g)\n", S->gravitation.x, S->gravitation.y);
		if (S->Finish < DBL_MAX)
			printf("Finish    = %g\n", S->Finish);


		if (S->save_dt < DBL_MAX)
			printf("dt_save = %g\n", double(S->save_dt));
		if (S->streak_dt < DBL_MAX)
			printf("dt_streak = %g\n", double(S->streak_dt));
		if (S->profile_dt < DBL_MAX)
			printf("dt_profile = %g\n", double(S->profile_dt));

			printf("\n");
		if (S->VortexList->size_safe())
			printf(" 1: Vortexes [%ld]\n", S->VortexList->size_safe());
		if (S->HeatList->size_safe())
			printf(" 2: Heat     [%ld]\n", S->HeatList->size_safe());
		if (S->StreakSourceList->size_safe())
			printf(" 3: StrkSrc  [%ld]\n", S->StreakSourceList->size_safe());
		if (S->StreakList->size_safe())
			printf(" 4: Streak   [%ld]\n", S->StreakList->size_safe());
		for (int i=0; i<S->BodyList->size_safe(); i++)
		{
			printf(" %d: BData   \n", 5+i*2);
			printf(" %d: Body    [%ld]\n", 5+i*2+1, S->BodyList->at(i)->List->size_safe());
		}

		return 0;
	}

	TBody *body;
	switch (atoi(argv[2]))
	{
		case 1:
			const_for(S->VortexList, lobj)
				printf("%lf\t %lf\t %le\n", lobj->r.x, lobj->r.y, lobj->g);
			break;
		case 2:
			const_for(S->HeatList, lobj)
				printf("%lf\t %lf\t %le\n", lobj->r.x, lobj->r.y, lobj->g);
			break;
		case 3:
			const_for(S->StreakSourceList, lobj)
				printf("%lf\t %lf\t %lg\n", lobj->r.x, lobj->r.y, lobj->g);
			break;
		case 4:
			const_for(S->StreakList, lobj)
				printf("%lf\t %lf\t %lg\n", lobj->r.x, lobj->r.y, lobj->g);
			break;
		default:
			int arg = atoi(argv[2])-5;
			if (! (arg%2))
			{
				if (arg/2 >= S->BodyList->size_safe()) { fprintf(stderr, "No such entry\n"); return -1; }
				body = S->BodyList->at(arg/2);
				if (!body->pos.iszero())
					printf("Position   = (%lg, %lg, %lf (%.1lf deg))\n", body->pos.r.x,
					                                                     body->pos.r.y,
					                                                     body->pos.o,
					                                                     body->pos.o/3.141592*180.0);
				if (!body->dPos.iszero())
					printf("deltaPos   = (%lg, %lg, %lf (%.1lf deg))\n", body->dPos.r.x,
					                                                     body->dPos.r.y,
					                                                     body->dPos.o,
					                                                     body->dPos.o/3.141592*180.0);
				if ((body->k.r.x>=0) || (body->k.r.y>=0) || (body->k.o>=0))
					printf("spring_k   = (%lg, %lg, %lg)\n", body->k.r.x, body->k.r.y, body->k.o);
				if (strlen(body->SpeedX->getScript()))
					printf("SpeedX     = %s\n", body->SpeedX->getScript());
				if (strlen(body->SpeedY->getScript()))
					printf("SpeedY     = %s\n", body->SpeedY->getScript());
				if (strlen(body->SpeedO->getScript()))
					printf("SpeedO     = %s\n", body->SpeedO->getScript());
					printf("Speed_slae = (%lg, %lg, %lg)\n", body->Speed_slae.r.x,
					                                         body->Speed_slae.r.y,
					                                         body->Speed_slae.o);
					printf("Area       = %lg\n", body->getArea());
					printf("Com        = (%lg, %lg)\n", body->getCom().x, body->getCom().y);
					printf("Moi_c      = %lg\n", body->getMoi_c());
					printf("density    = %lg\n", body->density);
					printf("Force_born = (%lg, %lg, %lg)\n", body->Force_born.r.x, body->Force_born.r.y, body->Force_born.o);
					printf("Force_dead = (%lg, %lg, %lg)\n", body->Force_dead.r.x, body->Force_dead.r.y, body->Force_dead.o);
			} else
			{
				if (arg/2 >= S->BodyList->size_safe()) { fprintf(stderr, "No such entry\n"); return -1; }
				const_for(S->BodyList->at(arg/2)->List, latt)
					printf("%lf\t %lf\t %le\t %c\t %c\t %lg\n", latt->corner.x, latt->corner.y, latt->g,
					                                            latt->bc, latt->hc, latt->heat_const);
			}
			break;
	}

	return 0;
}
