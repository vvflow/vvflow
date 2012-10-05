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
			printf("Time  = %g\n", S->Time);
			printf("dt    = %g\n", S->dt);
			printf("Re    = %g\n", S->Re);
		if (S->Pr)
			printf("Pr    = %g\n", S->Pr);

		if (strlen(S->InfSpeedX->getScript()))
			printf("InfSpeedX = %s\n", S->InfSpeedX->getScript());
		if (strlen(S->InfSpeedY->getScript()))
			printf("InfSpeedY = %s\n", S->InfSpeedY->getScript());
			printf("InfSpeed  = (%g, %g)\n", S->InfSpeed().rx, S->InfSpeed().ry);
			printf("InfMarker = (%g, %g)\n", S->InfMarker.rx, S->InfMarker.ry);
		if (S->InfCirculation)
			printf("InfGamma  = %g\n", S->InfCirculation);
		if (!S->gravitation.iszero())
			printf("gravitation = (%g, %g)\n", S->gravitation.rx, S->gravitation.ry);
		if (S->Finish < DBL_MAX)
			printf("Finish    = %g\n", S->Finish);


		if (S->save_dt < DBL_MAX)
			printf("dt_save = %g\n", S->save_dt);
		if (S->streak_dt < DBL_MAX)
			printf("dt_streak = %g\n", S->streak_dt);
		if (S->profile_dt < DBL_MAX)
			printf("dt_profile = %g\n", S->profile_dt);

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
				printf("%lf\t %lf\t %lf\n", lobj->rx, lobj->ry, lobj->g);
			break;
		case 2:
			const_for(S->HeatList, lobj)
				printf("%lf\t %lf\t %lf\n", lobj->rx, lobj->ry, lobj->g);
			break;
		case 3:
			const_for(S->StreakSourceList, lobj)
				printf("%lf\t %lf\t %lf\n", lobj->rx, lobj->ry, lobj->g);
			break;
		case 4:
			const_for(S->StreakList, lobj)
				printf("%lf\t %lf\t %lf\n", lobj->rx, lobj->ry, lobj->g);
			break;
		default:
			if (atoi(argv[2]) % 2)
			{
				body = S->BodyList->at(0);
				if (body->Position.rx || body->Position.ry || body->Angle)
					printf("Position   = (%lg, %lg, %lg)\n", body->Position.rx, body->Position.ry, body->Angle);
				if (body->deltaPosition.rx || body->deltaPosition.ry || body->deltaAngle)
					printf("deltaPos   = (%lg, %lg, %lg)\n", body->deltaPosition.rx, body->deltaPosition.ry, body->deltaAngle);
				if ((body->kx>=0) || (body->ky>=0) || (body->ka>=0))
					printf("spring_k   = (%lg, %lg, %lg)\n", body->kx, body->ky, body->ka);
				if (strlen(body->SpeedX->getScript()))
					printf("SpeedX     = %s\n", body->SpeedX->getScript());
				if (strlen(body->SpeedY->getScript()))
					printf("SpeedY     = %s\n", body->SpeedY->getScript());
				if (strlen(body->SpeedO->getScript()))
					printf("SpeedO     = %s\n", body->SpeedO->getScript());
					printf("Area       = %lg\n", body->getArea());
					printf("Com        = (%lg, %lg)\n", body->getCom().rx, body->getCom().ry);
					printf("Moi_c      = %lg\n", body->getMoi_c());
					printf("density    = %lg\n", body->density);		
			} else
			{
				const_for(S->BodyList->at(0)->List, latt)
					printf("%lg\t %lg\t %lg\t %c\t %c\t %lg\n", latt->corner.rx, latt->corner.ry, latt->g,
					                                            latt->bc, latt->hc, latt->heat_const);
			}
			break;
	}

	return 0;
}
