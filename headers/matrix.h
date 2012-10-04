#ifndef matrix_h
#define matrix_h

class Matrix
{
	public:
		Matrix(int size);
		size_t size;

		double *objectAtIndex(int eq, int j);
		double **solutionAtIndex(int i);
		double *rightColAtIndex(int i);
		void markBodyMatrixAsFilled() {bodyMatrixIsOk_ = true;}
		void spoilMatrix() {bodyMatrixIsOk_ = inverseMatrixIsOk_ = false;}
		void solveUsingInverseMatrix(bool useInverseMatrix);

	private:
		void transpose(double* A, int N);
		void FillInverseMatrix();

	private:
		double* BodyMatrix;
		double* InverseMatrix;
		double* RightCol;
		double** solution;
		int *ipvt; //technical variable for lapack

		bool bodyMatrixIsOk_;
		bool inverseMatrixIsOk_;

	public:
		bool bodyMatrixIsOk() {return bodyMatrixIsOk_;}
		bool inverseMatrixIsOk() {return inverseMatrixIsOk_;}
	//Source for saving and loading matrix can be found
	//in git baecb9308569b6f52a863697908c33ced7ed811b
};

#endif
