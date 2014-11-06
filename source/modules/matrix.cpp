#include "mkl.h"
#include "stdio.h"
#include "string.h"
#include "matrix.h"

#define NDEBUG
#include "assert.h"

#define sqr(x) x*x

Matrix::Matrix(int size_)
{
	size = size_;

	BodyMatrix = new double[sqr(size)]; //(double*)malloc(N*N*sizeof(double));
	InverseMatrix = new double[sqr(size)]; //(double*)malloc(N*N*sizeof(double));
	RightCol = new double[size]; //(double*)malloc(N*sizeof(double));
	solution = new double*[size]; //(double*)malloc(N*sizeof(double));
	ipvt = new int[size+1]; //(int*)malloc((N+1)*sizeof(int));

	bodyMatrixIsOk_ = false;
	inverseMatrixIsOk_ = false;
}

double* Matrix::objectAtIndex(int i, int j)
{
	if ( (i>=size) || (j>=size) ) return NULL;
	return BodyMatrix + i*size + j;
}

double** Matrix::solutionAtIndex(int i)
{
	if ( i>=size ) return NULL;
	return solution + i;
}

double* Matrix::rightColAtIndex(int i)
{
	if ( i>=size ) return NULL;
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
		for (size_t i=0; i<size; i++)
		{
			double *RowI = InverseMatrix + size*i;

			if (!solution[i])
			{
				fprintf(stderr, "Warning: solution[%ld] = NULL\n", i);
				continue;
			}

			*solution[i] = 0;
			for (size_t j=0; j<size; j++)
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

		int info, one=1, N_int=size;
		transpose(BodyMatrix, size);
		dgesv_(&N_int,&one,BodyMatrix,&N_int,ipvt,RightCol,&N_int,&info);

		if (info)
		{
			fprintf(stderr, "dgesv_() failed with info = %d\n", info);
			exit(-2);
		}

		for (size_t i=0; i<size; i++)
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

	memcpy(InverseMatrix, BodyMatrix, sqr(size)*sizeof(double));

	int size_int = size;
	int LWORK = sqr(size);
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

void Matrix::transpose(double* A, int N)
{
	#pragma omp parallel for
	for (int i=0; i<size; i++)
	{
		for (int j=i+1; j<size; j++)
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

	for (int i=0; i<size; i++)
	{
		for (int j=0; j<size; j++)
		{
			fprintf(fout, "%lf\t", BodyMatrix[i*size+j]);
		}
		fprintf(fout, "\t%lf\n", RightCol[i]);
	}
	fclose(fout);
}

void Matrix::fillWithZeros()
{
	bodyMatrixIsOk_ = 0;
	memset(BodyMatrix, 0, sizeof(double)*sqr(size));
	memset(RightCol, 0, sizeof(double)*size);
}

bool Matrix::testNan()
{
	if (!bodyMatrixIsOk_) { perror("Test Nan: matrix is not filled"); return false; }
	bool foundNan = false;
	for (int i=0; i<size; i++)
	{
		for (int j=0; j<size; j++)
		{
			double el = BodyMatrix[i*size+j];
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
