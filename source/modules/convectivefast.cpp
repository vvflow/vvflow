#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
using namespace std;

#include "convectivefast.h"
//#define dbg(func) cout << "Doing " << #func << "... " << flush; func; cout << "done\n";
#define dbg(func) func;

/********************************** HEADER ************************************/

extern "C" {
	void dgesv_(int* n, const int* nrhs, double* a, int* lda, int* ipiv,
	            double *x, int *incx, int *info);
	void dgetrf_(int* M, int *N, double* A, int* lda, int* IPIV, int* INFO);
	void dgetri_(int* N, double* A, int* lda, int* IPIV, double* WORK,
	             int* lwork, int* INFO);
}

namespace {

Space *S;
double Rd2; //radius of discrete ^2
double Rd;

inline
TVec BioSavar(const TObj &obj, const TVec &p);
TVec SpeedSum(const TNode &Node, const TVec &p);

TVec BoundaryConvective(const TBody &b, const TVec &p);

int N; // BodySize;
double *BodyMatrix;
double *InverseMatrix;
double *RightCol;
double *Solution;
int *ipvt; //technical variable for lapack
void inverse(double* A, int N, double *workspace); //workspace is being spoiled;
void transpose(double* A, int N);
void SolveMatrix_inv();

bool BodyMatrixOK;
bool InverseMatrixOK;

double ObjectInfluence(TObj &obj, TObj &seg1, TObj &seg2, double rd);
double NodeInfluence(TNode &Node, TObj &seg1, TObj &seg2, double rd);
double AttachInfluence(TObj &seg1, TObj &seg2, const TAtt &center, double rd);
extern"C"{
void fortobjectinfluence_(double *x, double *y, double *x1, double *y1,
					double *x2, double *y2, double *ax, double *ay, double *eps);
}

int LoadMatrix(double *matrix, const char* filename);
void SaveMatrix(double *matrix, const char* filename);
int LoadMatrix_bin(double *matrix, const char* filename);
void SaveMatrix_bin(double *matrix, const char* filename);

}

/****************************** MAIN FUNCTIONS ********************************/

void InitConvectiveFast(Space *sS, double sRd2)
{
	S = sS;
	Rd2 = sRd2;
	Rd = sqrt(Rd2);

	N = 0; const_for(sS->BodyList, llbody){ N+=(**llbody).List->size(); }
	BodyMatrix = (double*)malloc(N*N*sizeof(double));
	InverseMatrix = (double*)malloc(N*N*sizeof(double));
	RightCol = (double*)malloc(N*sizeof(double));
	Solution = (double*)malloc(N*sizeof(double));
	ipvt = (int*)malloc((N+1)*sizeof(int));
	BodyMatrixOK = InverseMatrixOK = false;
}

double *MatrixLink() { return BodyMatrix; }

TVec SpeedSumFast(TVec p)
{
	TVec res(0, 0);
	TNode* Node = FindNode(p);
	if (!Node) return res;

	const_for (Node->FarNodes, llfnode)
	{
		res+= BioSavar((**llfnode).CMp, p) + BioSavar((**llfnode).CMm, p);
	}
	res *= C_1_2PI;
	res += SpeedSum(*Node, p);

	return res;
}

namespace {
inline
TVec BioSavar(const TObj &obj, const TVec &p)
{
	TVec dr = p - obj;
	return rotl(dr)*(obj.g / (dr.abs2() + Rd2) );
}}


namespace {
TVec SpeedSum(const TNode &Node, const TVec &p)
{
	TVec dr, res(0, 0);

	const_for (Node.NearNodes, llnnode)
	{
		auto vlist = (**llnnode).VortexLList;
		if ( !vlist ) { continue; }

		const_for (vlist, llobj)
		{
			if (!*llobj) continue;
			res+= BioSavar(**llobj, p); 
		}
	}

	res *= C_1_2PI;
	return res;
}}

void CalcConvectiveFast()
{
	if (!S) {cerr << "CalcConvectiveFast() is called before initialization"
	              << endl; return; }

	double Teilor1, Teilor2, Teilor3, Teilor4;

	auto BottomNodes = GetTreeBottomNodes();
	if (!BottomNodes) {cerr << "CalcConvectiveFast() is called before tree is built"
	                        << endl; return; }

	#pragma omp parallel for
	const_for (BottomNodes, llbnode)
	{
		TNode &bnode = **llbnode;

		TVec DistP, DistM; //Distance between current node center and positive center of mass of far node 
		double FuncP1, FuncM1; //Extremely complicated useless variables
		double FuncP2, FuncM2;

		Teilor1 = Teilor2 = Teilor3 = Teilor4 = 0;

		const_for (bnode.FarNodes, llfnode)
		{
			TNode &fnode = **llfnode;
			DistP = TVec(bnode.x, bnode.y) - fnode.CMp;
			DistM = TVec(bnode.x, bnode.y) - fnode.CMm;

			double _1_DistPabs = 1/DistP.abs2();
			double _1_DistMabs = 1/DistM.abs2();
			FuncP1 = fnode.CMp.g * _1_DistPabs; //Extremely complicated useless variables
			FuncM1 = fnode.CMm.g * _1_DistMabs;
			FuncP2 = FuncP1 * _1_DistPabs;
			FuncM2 = FuncM1 * _1_DistMabs;
			
			Teilor1 -= (FuncP1*DistP.ry + FuncM1*DistM.ry);
			Teilor2 += (FuncP1*DistP.rx + FuncM1*DistM.rx);
			Teilor3 += (FuncP2*DistP.ry*DistP.rx + FuncM2*DistM.ry*DistM.rx);
			Teilor4 += (FuncP2 * (DistP.ry*DistP.ry - DistP.rx*DistP.rx) + FuncM2 * (DistM.ry*DistM.ry - DistM.rx*DistM.rx));
		}

		Teilor1 *= C_1_2PI;
		Teilor2 *= C_1_2PI;
		Teilor3 *= C_1_PI;
		Teilor4 *= C_1_2PI;

		TVec dr_local;
		
		if (bnode.VortexLList)
		{
			const_for (bnode.VortexLList, llobj)
			{
				if (!*llobj) {continue;}
				TObj &obj = **llobj;
				dr_local = obj - TVec(bnode.x, bnode.y);
				obj.v += TVec(Teilor1, Teilor2) + S->InfSpeed() + SpeedSum(bnode, obj) +
				         TVec(TVec(Teilor3,  Teilor4)*dr_local, 
				              TVec(Teilor4, -Teilor3)*dr_local);
			}
		}

		if (bnode.HeatLList)
		{
			const_for (bnode.HeatLList, llobj)
			{
				TObj &obj = **llobj;
				dr_local = obj - TVec(bnode.x, bnode.y);
				obj.v += TVec(Teilor1, Teilor2) + S->InfSpeed() + SpeedSum(bnode, obj) +
				         TVec(TVec(Teilor3,  Teilor4)*dr_local, 
				              TVec(Teilor4, -Teilor3)*dr_local);
			}
		}
	}
}

void CalcBoundaryConvective()
{
	auto vlist = S->VortexList;
	//TList<TObj> *hlist = ConvectiveFast_S->HeatList;

	if (!vlist) { return; }

	const_for(vlist, lobj)
	{
		const_for(S->BodyList, llbody)
		{
			lobj->v += BoundaryConvective(**llbody, *lobj)*C_1_2PI;
		}
	}
}

namespace {
TVec BoundaryConvective(const TBody &b, const TVec &p)
{
	TVec dr, res(0, 0);
	auto alist = b.AttachList;
	double rotspeed = b.RotSpeed(S->Time);
	if (!rotspeed) return res;

	const_for(alist, latt)
	{
		dr = p - *latt;
		res += (dr*latt->q + rotl(dr)*latt->g) * (rotspeed/( dr.abs2() + Rd2 ));
		if (latt->bc == bc::slip)
		{
			res += BioSavar(*b.next(b.obj(latt)), p);
		}
	}

	return res;
}}

void CalcCirculationFast(bool use_inverse)
{
	dbg(FillRightCol());
	if (!BodyMatrixOK) {dbg(FillMatrix());}
	if(use_inverse)
	{
		if (!InverseMatrixOK)
		{
			memcpy(InverseMatrix, BodyMatrix, N*N*sizeof(double));
			dbg(inverse(InverseMatrix, N, BodyMatrix));
			InverseMatrixOK = true;
			BodyMatrixOK = false;
		}
		dbg(SolveMatrix_inv());
	}
	else
	{
		dbg(SolveMatrix());
		BodyMatrixOK = false;
	}

	const_for (S->BodyList, llbody)
	{
		TBody &body = **llbody;
		const_for(body.List, lobj)
		{
			lobj->g = Solution[body.att(lobj)->eq_no];
		}
	}
}


namespace {
//inline
double ObjectInfluence(TObj &obj, TObj &seg1, TObj &seg2, double rd)
{
	TVec res;
	fortobjectinfluence_(&obj.rx, &obj.ry, &seg1.rx, &seg1.ry,
	                                 &seg2.rx, &seg2.ry, &res.rx, &res.ry, &rd);
	TVec dl=seg2-seg1;
	return -(rotl(res)*dl)/dl.abs()*C_1_2PI;
	//FIXME kill fortran
}}

namespace {
//inline
double NodeInfluence(TNode &Node, TObj &seg1, TObj &seg2, double rd)
{
	TVec res(0, 0), tmp;

	const_for(Node.NearNodes, llnnode)
	{
		auto vlist = (**llnnode).VortexLList;
		if ( !vlist ) { continue; }

		const_for (vlist, llobj)
		{
			if (!*llobj) {continue;}
			TObj &obj = **llobj;
			fortobjectinfluence_(&obj.rx, &obj.ry, &seg1.rx, &seg1.ry,
			                         &seg2.rx, &seg2.ry, &tmp.rx, &tmp.ry, &rd);
			res+= tmp*obj.g;
		}
	}

	const_for(Node.FarNodes, llfnode)
	{
		TNode &fnode = **llfnode;

		fortobjectinfluence_(&fnode.CMp.rx, &fnode.CMp.ry, &seg1.rx,
		                   &seg1.ry, &seg2.rx, &seg2.ry, &tmp.rx, &tmp.ry, &rd);
		res+= tmp*fnode.CMp.g;
		fortobjectinfluence_(&fnode.CMm.rx, &fnode.CMm.ry, &seg1.rx,
		                   &seg1.ry, &seg2.rx, &seg2.ry, &tmp.rx, &tmp.ry, &rd);
		res+= tmp*fnode.CMm.g;
	}

	TVec dl=seg2-seg1;
	return -(rotl(res)*dl)/dl.abs()*C_1_2PI;
}}

namespace {
//inline
double AttachInfluence(TObj &seg1, TObj &seg2, const TAtt &center, double rd)
{
	TVec res(0,0), tmp;

	const_for(S->BodyList, llbody)
	{
		TBody &body = **llbody;
		if (!body.AttachList->size_safe()) continue;
		const_for(body.AttachList, latt)
		{
			if (latt == &center) continue;
			fortobjectinfluence_(&latt->rx, &latt->ry, &seg1.rx, &seg1.ry,
			                         &seg2.rx, &seg2.ry, &tmp.rx, &tmp.ry, &rd);
			res+= tmp*latt->g - rotl(tmp)*latt->q;
		}
	}
	TVec dl=seg2-seg1;
	return -(rotl(res)*dl)/dl.abs()*C_1_2PI - center.q*0.5; //FIXME sign
	//FIXME kill fortran
}}

void FillMatrix()
{
	//BodyMatrix[N*i+j]
	//RightCol[i]

	const_for(S->BodyList, llbody)
	{
		TBody &body = **llbody;
		const_for(body.List, lbvort)
		{
			//temporarily vort->g stores eps info.
			lbvort->g = (*body.next(lbvort)-*lbvort).abs()+
			            (*body.prev(lbvort)-*lbvort).abs();
			lbvort->g *= 0.25;
		}
	}

	const_for(S->BodyList, llibody)
	{
		TBody &ibody = **llibody;
		#pragma omp parallel for

		const_for(ibody.AttachList, latt)
		{
			int i = latt->eq_no;
			const_for(S->BodyList, lljbody)
			{
				TBody &jbody = **lljbody;
				const_for(jbody.List, lobj)
				{
					int j=jbody.att(lobj)->eq_no;
					switch (latt->bc)
					{
					case bc::slip:
					case bc::noslip:
						BodyMatrix[N*i+j] = ObjectInfluence(*lobj, *ibody.obj(latt),
						            *ibody.next(ibody.obj(latt)), lobj->g);
						break;
					case bc::kutta:
						BodyMatrix[N*i+j] = (lobj == ibody.obj(latt)) ||
						                    (lobj == ibody.next(ibody.obj(latt))) ?
						                    1:0;
					case bc::noperturbations:
						BodyMatrix[N*i+j] = (llibody==lljbody)?1:0;
						break;
					case bc::tricky:
						BodyMatrix[N*i+j] = 1;
						break;
					}
				}
			}
		}
	}

	BodyMatrixOK = true;
}

void FillRightCol()
{
	double rot_sum = 0;
	const_for (S->BodyList, llbody)
	{
		TBody &body = **llbody;
		double tmp = 0;
		const_for(body.AttachList, latt)
		{
			tmp+= latt->g;
		}
		tmp*= body.RotSpeed(S->Time);
		rot_sum+= tmp;
	}

	const_for (S->BodyList, llbody)
	{
		TBody &body = **llbody;
		double tmp = 0;
		const_for(body.AttachList, latt)
		{
			tmp+= latt->g;
		}
		tmp*= body.RotSpeed(S->Time) - body.RotSpeed(S->Time-S->dt);

		#pragma omp parallel for
		const_for (body.AttachList, latt)
		{
			TNode* Node = FindNode(*latt);
			if (!Node) { continue; }

			TVec SegDl = latt->dl;

			switch (latt->bc)
			{
			case bc::slip:
			case bc::noslip:
			RightCol[latt->eq_no] = rotl(S->InfSpeed())*SegDl;
			RightCol[latt->eq_no] -= NodeInfluence(*Node, *body.obj(latt),
			                                    *body.next(body.obj(latt)), Rd);
			const_for (S->BodyList, lljbody)
			{
				double RotSpeed_tmp = (**lljbody).RotSpeed(S->Time);
				if (!RotSpeed_tmp) continue;
				RightCol[latt->eq_no] -= AttachInfluence(*body.obj(latt),
				                          *body.next(body.obj(latt)), *latt, Rd)
				                     * RotSpeed_tmp;
			}
			break;
			case bc::kutta:
			case bc::noperturbations:
			RightCol[latt->eq_no] = -tmp +  body.g_dead;
			body.g_dead = 0;
			break;
			case bc::tricky:
			RightCol[latt->eq_no] = -S->gsum() - rot_sum;
//			cerr << "\n\t\t\t\t\t2 " << RightCol[latt->eq_no] << " \t" << rot_sum << "\t" << S->gsum() << endl;
			break;
			}
		}
	}
}

void SolveMatrix()
{
	if (!BodyMatrixOK)
	{
		cerr << "Matrix isn't filled!\n";
		return;
	}

	int info, one=1;
	transpose(BodyMatrix, N);
	dgesv_(&N,&one,BodyMatrix,&N,ipvt,RightCol,&N,&info);

	if (info)
	{
		cerr << "SolveMatrix() failed with info=" << info << endl;
		return;
	}

	for (int i=0; i<N; i++)
	{
		Solution[i] = RightCol[i];
	}
}

void SpoilBodyMatrix() {BodyMatrixOK=false;}
void SpoilInverseMatrix() {InverseMatrixOK=false;}

namespace {
void inverse(double* A, int N, double* workspace)
{
	int LWORK = N*N;
	int info;

	dgetrf_(&N,&N,A,&N,ipvt,&info);
	if (info) { cerr << "dgetrf_() failed with info=" << info << endl; return; }
	dgetri_(&N,A,&N,ipvt,workspace,&LWORK,&info);
	if (info) { cerr << "dgetri_() failed with info=" << info << endl; return; }
}}

namespace {
void transpose(double* A, int N)
{
	#pragma omp parallel for
	for (int i=0; i<N; i++)
	{
		for (int j=i+1; j<N; j++)
		{
			double tmp = A[i*N+j];
			A[i*N+j] = A[j*N+i];
			A[j*N+i] = tmp;
		}
	}
}}

namespace {
void SolveMatrix_inv()
{
	if (!InverseMatrixOK)
	{
		cerr << "Inverse!\n";
	}

	#pragma omp parallel for
	for (int i=0; i<N; i++)
	{
		double *RowI = InverseMatrix + N*i;
		double &SolI = Solution[i];
		SolI = 0;
		for (int j=0; j<N; j++)
		{
			SolI+= RowI[j]*RightCol[j];
		}
	}
}}

/****** LOAD/SAVE MATRIX *******/

int LoadBodyMatrix(const char* filename)
{ return BodyMatrixOK = (LoadMatrix(BodyMatrix, filename) == N*N); }
int LoadInverseMatrix(const char* filename)
{ InverseMatrixOK = (LoadMatrix(InverseMatrix, filename) == N*N); return InverseMatrixOK; }
void SaveBodyMatrix(const char* filename)
{ SaveMatrix(BodyMatrix, filename); }
void SaveInverseMatrix(const char* filename)
{ SaveMatrix(InverseMatrix, filename); }

namespace {
int LoadMatrix(double *matrix, const char* filename)
{
	FILE *fin;

	fin = fopen(filename, "r");
	if (!fin) { cerr << "No file called \'" << filename << "\'\n"; return -1; } 
	double *dst = matrix;
	while ( fscanf(fin, "%lf", dst)==1 )
	{
		dst++;
	}
	fclose(fin);
	return (dst - matrix);
}}

namespace {
void SaveMatrix(double *matrix, const char* filename)
{
	FILE *fout;

	fout = fopen(filename, "w");
	if (!fout) { cerr << "Error opening file \'" << filename << "\'\n"; return; } 
	double *src = matrix;
	for (int i=0; i<N*N; i++)
	{
		fprintf(fout, "%f ", src[i]);
		if (!((i+1)%N)) fprintf(fout, "\n");
	}
	fclose(fout);
}}

/****** BINARY LOAD/SAVE *******/

int LoadBodyMatrix_bin(const char* filename)
{ BodyMatrixOK = (LoadMatrix_bin(BodyMatrix, filename) == N*N); return BodyMatrixOK; }
int LoadInverseMatrix_bin(const char* filename)
{ InverseMatrixOK = (LoadMatrix_bin(InverseMatrix, filename) == N*N); return InverseMatrixOK; }
void SaveBodyMatrix_bin(const char* filename)
{ SaveMatrix_bin(BodyMatrix, filename); }
void SaveInverseMatrix_bin(const char* filename)
{ SaveMatrix_bin(InverseMatrix, filename); }

namespace {
int LoadMatrix_bin(double *matrix, const char* filename)
{
	FILE *fin;
	int result;

	fin = fopen(filename, "rb");
	if (!fin) { return -1; }
	result = fread(matrix, sizeof(double), N*N, fin);
	fclose(fin);

	return result;
}}

namespace {
void SaveMatrix_bin(double *matrix, const char* filename)
{
	FILE *fout;

	fout = fopen(filename, "wb");
	fwrite(matrix, sizeof(double), N*N, fout);
	fclose(fout);
}}

