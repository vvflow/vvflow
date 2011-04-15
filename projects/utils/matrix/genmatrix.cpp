#include "iostream"
#include "stdio.h"
#include "libVVHD/core.h"
#include "libVVHD/convectivefast.h"
using namespace std;

int main(int argc, char** argv)
{
	Space *S = new Space(false, false, NULL, NULL);
	S->Body->LoadFromFile(argv[1]);
	char fname[32]; sprintf(fname, "%s.matrix", argv[1]);
	InitConvectiveFast(S, 1E-4);

	cout << "Body size: " << S->Body->List->size << endl;

	FillMatrix();
	SaveBodyMatrix_bin(fname);
	return 0;
}
