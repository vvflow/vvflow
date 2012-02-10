#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	double t = (argc>=2)?atof(argv[1]):1.;
	double c = (argc>=3)?atof(argv[2]):1.;
	printf("%g\n", t<c?t/c:1);
	return 0;
}
