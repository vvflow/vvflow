#ifndef EXPFAST_H
#define EXPFAST_H

int InitExpFast(double Eps);
double expfast(double x);

class ExpFast
{
	public:
		ExpFast(double sEps);
		double expf(double x /*x<0*/);
		~ExpFast();
	private:
		double Eps;
		double maxarg;
		double *Array;
		int size; //size of Array[]
		double dx;
		double dx1; // =1/dx
};

#endif
