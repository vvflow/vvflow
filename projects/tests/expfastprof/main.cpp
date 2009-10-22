#include "libVVHD/core.h"
#include "math.h"
#include "iostream"
#include <time.h>

using namespace std;

int main()
{
	time_t start, stop;
	InitExpFast(1E-5);
	cout << exp(-7) << " " << expfast(-7) << endl;

	time(&start);
	for (int i=0; i<1E9; i++)
	{
		exp(-10.);
		//expfast(-10.);
	}
	time(&stop);
	cout << "\nTotal time: " << (stop-start) << endl;

	return 0;
}
