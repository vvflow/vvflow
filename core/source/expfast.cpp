#include <math.h>
#include <iostream>
#include "expfast.h"

using namespace std;

namespace {
	double ExpFast_Eps;
	double ExpFast_MaxArg;
	double* ExpFast_Array;
	int ExpFast_ArraySize; //size of Array[]
	double ExpFast_dx;
	double ExpFast_dx1; // =1/dx
}

int InitExpFast(double sEps)
{
	ExpFast_Eps = sEps;
	ExpFast_dx = sqrt(8*ExpFast_Eps);
	ExpFast_dx1 = 1/ExpFast_dx;
	ExpFast_MaxArg = log(ExpFast_Eps);
	ExpFast_ArraySize = 1 - ExpFast_MaxArg*ExpFast_dx1;
	ExpFast_Array = new double[ExpFast_ArraySize];
	
	for(int i=0; i<ExpFast_ArraySize; i++)
	{
		ExpFast_Array[i] = exp(-i*ExpFast_dx);
	}
	ExpFast_Array[ExpFast_ArraySize-1] = 0;
}

double expfast(double x)
{
	if (x < ExpFast_MaxArg) asm("fldz\nleave\nret");
	if (x >= 0) return exp(x);

	int i= floor(-x*ExpFast_dx1);
	double *ArrayI = ExpFast_Array+i;
	return *ArrayI + (x+i*ExpFast_dx)*( *ArrayI - *(ArrayI+1))*ExpFast_dx1;
//	return *ArrayI-(x+i*dx)*(*(ArrayI+1)-*(ArrayI))*dx1;
	/*
	int i;
	asm(
		"fldl %1\n" // push x
		"fldl %2\n" // push dx1
		"fmulp %%st, %%st(1)\n"
		"fldl 0x0\n" // push 0.5
		"faddp %%st, %%st(1)\n"
		"fchs\n"
		"frndint\n"
		"fstp %0"
		: "=m" (i)
		: "m" (x), "m" (ExpFast_dx1), "g" (0.5)
		: "st");
//	int i= floor(-x*dx1-0.5);
	//double *ArrayI = Array+i;
	//return Array[i] + (x+i*dx)*(Array[i]-Array[i+1])*dx1;
	//return *ArrayI-(x+i*dx)*(*(ArrayI+1)-*(ArrayI))*dx1;
	
	
	asm(
		//double *ArrayI = Array+i; (%eax)
		"mov 	%0, %%edx\n"
		"mov 	%1, %%eax\n"
		"shl 	$0x3, %%eax\n"
		"lea 	(%%edx,%%eax,1), %%eax\n"
		: 
		: "m" (ExpFast_Array), "m" (i)
		: "eax", "edx"
		);
		//finally EAX == Array+i
	asm(
		//return *ArrayI-(x+i*dx)*(*(ArrayI+1)-*(ArrayI))*dx1;
		"fldl 	(%%eax)\n" 		//push Array[i]
		"fldl 	(%%eax)\n" 		//push Array[i]
		"add 	$0x8, %%eax\n" 	//ArrayI ++
		"fsubl 	(%%eax)\n" 		//st(0) = Array[i] - Array[i+1]
		"fild 	%0\n" 			//push i
		"mov 	%1, %%eax\n"	//EAX = this
		"fldl 	0x18(%%eax)\n"	//push dx
		"fmulp 	%%st, %%st(1)\n"//st(0) = i*dx
		"faddl 	%2\n"			//st(0) = i*dx+x
		"fmulp 	%%st, %%st(1)\n"
		"fldl 	0x20(%%eax)\n"	//push dx1
		"fmulp 	%%st, %%st(1)\n"
		//"fmulp 	%%st, %%st(1)\n"
		"faddp %%st, %%st(1)\n"
		: 
		: "m" (i), "g" (this), "m" (x)
		: "eax", "st"
		);
		
	asm("leave\nret");
	return 0;*/
}
/*
ExpFast::ExpFast(double sEps)
{
	Eps = sEps;
	dx = sqrt(8*Eps);
	dx1 = 1/dx;

	maxarg = log(Eps); //maxarg < 0
	size = 1-maxarg*dx1;
	Array = new double[size];

	for(int i=0; i<size; i++)
	{
		Array[i] = exp(-i*dx);
	}
	Array[size-1] = 0;
}*/
/*
double ExpFast::expf(double x)
{
	if (x < maxarg) return 0;
	int i=floor(-x*dx1);
	return Array[i]-(x+i*dx)*(Array[i+1]-Array[i])*dx1;
}
*//*
double ExpFast::expf(double x)
{
	if (x < maxarg) asm("fldz\nleave\nret");
	
	int i;
	asm(
		"fldl %1\n" // push x
		"fldl %2\n" // push dx1
		"fmulp %%st, %%st(1)\n"
		"fldl 0x0\n" // push 0.5
		"faddp %%st, %%st(1)\n"
		"fchs\n"
		"frndint\n"
		"fstp %0"
		: "=m" (i)
		: "m" (x), "m" (dx1), "g" (0.5)
		: "st");
//	int i= floor(-x*dx1-0.5);
	//double *ArrayI = Array+i;
	//return Array[i] + (x+i*dx)*(Array[i]-Array[i+1])*dx1;
	//return *ArrayI-(x+i*dx)*(*(ArrayI+1)-*(ArrayI))*dx1;
	
	
	asm(
		//double *ArrayI = Array+i; (%eax)
		"mov 	%0, %%edx\n"
		"mov 	%1, %%eax\n"
		"shl 	$0x3, %%eax\n"
		"lea 	(%%edx,%%eax,1), %%eax\n"
		: 
		: "m" (Array), "m" (i)
		: "eax", "edx"
		);
		//finally EAX == Array+i
	asm(
		//return *ArrayI-(x+i*dx)*(*(ArrayI+1)-*(ArrayI))*dx1;
		"fldl 	(%%eax)\n" 		//push Array[i]
		"fldl 	(%%eax)\n" 		//push Array[i]
		"add 	$0x8, %%eax\n" 	//ArrayI ++
		"fsubl 	(%%eax)\n" 		//st(0) = Array[i] - Array[i+1]
		"fild 	%0\n" 			//push i
		"mov 	%1, %%eax\n"	//EAX = this
		"fldl 	0x18(%%eax)\n"	//push dx
		"fmulp 	%%st, %%st(1)\n"//st(0) = i*dx
		"faddl 	%2\n"			//st(0) = i*dx+x
		"fmulp 	%%st, %%st(1)\n"
		"fldl 	0x20(%%eax)\n"	//push dx1
		"fmulp 	%%st, %%st(1)\n"
		//"fmulp 	%%st, %%st(1)\n"
		"faddp %%st, %%st(1)\n"
		: 
		: "m" (i), "g" (this), "m" (x)
		: "eax", "st"
		);
		
	asm("leave\nret");
	return 0;
}

ExpFast::~ExpFast()
{
	delete [] Array;
}*/
