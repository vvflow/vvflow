#include "stdio.h"
#include "string.h"
#include "malloc.h"
#include "stdlib.h"
#include "matrix.h"

#ifdef HAVE_MKL
#include "mkl.h"
#else
extern "C" {
	void dgesv_(int* n, const int* nrhs, double* a, int* lda, int* ipiv, double *x, int *incx, int *info);
	void dgetrf_(int* M, int *N, double* A, int* lda, int* IPIV, int* INFO);
	void dgetri_(int* N, double* A, int* lda, int* IPIV, double* WORK, int* lwork, int* INFO);
}
#endif

#define NDEBUG
#include "assert.h"

#define sqr(x) (x*x)

Matrix::Matrix()
{
	N = 0;

	BodyMatrix = InverseMatrix = RightCol = NULL;
	solution = NULL;
	ipvt = NULL;
	bodyMatrixIsOk_ = false;
	inverseMatrixIsOk_ = false;
}

Matrix::~Matrix()
{
	free(BodyMatrix);
	free(InverseMatrix);
	free(RightCol);
	free(solution);
	free(ipvt);
}

void Matrix::resize(unsigned newsize)
{
	if (newsize == N) return;
	N = newsize;

	BodyMatrix =    (double*)realloc(BodyMatrix,    sqr(N)*sizeof(double));
	InverseMatrix = (double*)realloc(InverseMatrix, sqr(N)*sizeof(double));
	RightCol =      (double*)realloc(RightCol,      N*sizeof(double));
	solution =     (double**)realloc(solution,      N*sizeof(double));
	ipvt =             (int*)realloc(ipvt,          (N+1)*sizeof(int));

	bodyMatrixIsOk_ = false;
	inverseMatrixIsOk_ = false;
}

double* Matrix::objectAtIndex(unsigned i, unsigned j)
{
	if ( (i>=N) || (j>=N) ) return NULL;
	return BodyMatrix + i*N + j;
}

double** Matrix::solutionAtIndex(unsigned i)
{
	if ( i>=N ) return NULL;
	return solution + i;
}

double* Matrix::rightColAtIndex(unsigned i)
{
	if ( i>=N ) return NULL;
	return RightCol + i;
}

void Matrix::solveUsingInverseMatrix(bool useInverseMatrix)
{
	assert(!testNan());
	if (useInverseMatrix)
	{
		if (!inverseMatrixIsOk_)
		{
			FillInverseMatrix();
		}

		#pragma omp parallel for
		for (unsigned i=0; i<N; i++)
		{
			double *RowI = InverseMatrix + N*i;

			if (!solution[i])
			{
				fprintf(stderr, "Warning: solution[%u] = NULL\n", i);
				continue;
			}

			*solution[i] = 0;
			for (unsigned j=0; j<N; j++)
			{
				*solution[i] += RowI[j]*RightCol[j];
			}
		}
	} else
	{
		if (!bodyMatrixIsOk_)
		{
			fprintf(stderr, "Cannot solve SLAE without bodyMatrix\n");
			exit(-2);
		}

		int info, one=1, N_int=N;
		transpose(BodyMatrix, N);
		dgesv_(&N_int,&one,BodyMatrix,&N_int,ipvt,RightCol,&N_int,&info);

		if (info)
		{
			fprintf(stderr, "dgesv_() failed with info = %d\n", info);
			exit(-2);
		}

		for (size_t i=0; i<N; i++)
		{
			if (!solution[i])
			{
				fprintf(stderr, "Warning: solution[%ld] = NULL\n", i);
				continue;
			}

			*solution[i] = RightCol[i];
		}

		bodyMatrixIsOk_ = false;
	}
}

void Matrix::FillInverseMatrix()
{
	if (!bodyMatrixIsOk_)
	{
		fprintf(stderr, "Cannot fill inverseMatrix without bodyMatrix\n");
		exit(-2);
	}

	memcpy(InverseMatrix, BodyMatrix, sqr(N)*sizeof(double));

	int size_int = N;
	int LWORK = sqr(N);
	int info;

	dgetrf_(&size_int,&size_int,InverseMatrix,&size_int,ipvt,&info);
	if (info)
	{
		fprintf(stderr, "dgetrf_() failed with info = %d\n", info);
		exit(-2);
	}

	dgetri_(&size_int,InverseMatrix,&size_int,ipvt,BodyMatrix,&LWORK,&info);
	if (info)
	{
		fprintf(stderr, "dgetri_() failed with info = %d\n", info);
		exit(-2);
	}

	inverseMatrixIsOk_ = true;
	bodyMatrixIsOk_ = false;
}

void Matrix::transpose(double* A, unsigned N)
{
	#pragma omp parallel for
	for (unsigned i=0; i<N; i++)
	{
		for (unsigned j=i+1; j<N; j++)
		{
			double tmp = A[i*N+j];
			A[i*N+j] = A[j*N+i];
			A[j*N+i] = tmp;
		}
	}
}

void Matrix::save(const char* filename)
{
	if (!bodyMatrixIsOk_) { perror("Matrix is not filled"); return; }
	FILE *fout = fopen(filename, "w");
	if (!fout) { perror("Error saving the matrix"); return; }

	for (unsigned i=0; i<N; i++)
	{
		for (unsigned j=0; j<N; j++)
		{
			fprintf(fout, "%lf\t", BodyMatrix[i*N+j]);
		}
		fprintf(fout, "\t%lf\n", RightCol[i]);
	}
	fclose(fout);
}

void Matrix::fillWithZeros()
{
	bodyMatrixIsOk_ = 0;
	memset(BodyMatrix, 0, sizeof(double)*sqr(N));
	memset(RightCol, 0, sizeof(double)*N);
}

bool Matrix::testNan()
{
	if (!bodyMatrixIsOk_) { perror("Test Nan: matrix is not filled"); return false; }
	bool foundNan = false;
	for (unsigned i=0; i<N; i++)
	{
		for (unsigned j=0; j<N; j++)
		{
			double el = BodyMatrix[i*N+j];
			if (el != el)
			{
				fprintf(stderr, "Nan element in matrix: row %d, col %d\n", i, j);
				foundNan = true;
			}
		}

			double el = RightCol[i];
			if (el != el)
			{
				fprintf(stderr, "Nan element in rightcol: row %d\n", i);
				foundNan = true;
			}
	}

	return foundNan;
}
