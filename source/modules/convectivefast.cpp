#include <stdio.h>
#include <stdlib.h>
#include <iostream>
using namespace std;

#include "convectivefast.h"

/********************* HEADER ****************************/

namespace {

Space *ConvectiveFast_S;
double ConvectiveFast_Eps;

inline
TVec BioSavar(const TObj &obj, const TVec &p);
TVec SpeedSum(const TNode &Node, const TVec &p);

TVec BoundaryConvective(const TVec &p);

int N; // BodySize;
double *BodyMatrix;
double *InverseMatrix;
double *RightCol;
double *Solution;

bool BodyMatrixOK;
bool InverseMatrixOK; 

double ObjectInfluence(TObj &obj, TObj &seg1, TObj &seg2, double eps);
double NodeInfluence(TNode &Node, TObj &seg1, TObj &seg2, double eps);
double AttachInfluence(TObj &seg1, TObj &seg2, const TAtt &center, double eps);
extern"C"{
void fortobjectinfluence_(double *x, double *y, double *x1, double *y1,
					double *x2, double *y2, double *ax, double *ay, double *eps);
}

int LoadMatrix(double *matrix, const char* filename);
void SaveMatrix(double *matrix, const char* filename);
int LoadMatrix_bin(double *matrix, const char* filename);
void SaveMatrix_bin(double *matrix, const char* filename);

} //end of namespace

double *MatrixLink() { return BodyMatrix; }

/********************* SOURCE *****************************/

int InitConvectiveFast(Space *sS, double sEps)
{
	ConvectiveFast_S = sS;
	ConvectiveFast_Eps = sEps;

	N = sS->Body->List->size();
	BodyMatrix = (double*)malloc(N*N*sizeof(double));
	InverseMatrix = (double*)malloc(N*N*sizeof(double));
	RightCol = (double*)malloc(N*sizeof(double));
	Solution = (double*)malloc(N*sizeof(double));
	BodyMatrixOK = InverseMatrixOK = false;
	return 0;
}

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
	return rotl(dr)*(obj.g / (dr.abs2() + ConvectiveFast_Eps) );
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
			res+= BioSavar(**llobj, p); 
		}
	}

	res *= C_1_2PI;
	return res;
}}

int CalcConvectiveFast()
{
	double Teilor1, Teilor2, Teilor3, Teilor4;

	TVec infspeed = TVec(ConvectiveFast_S->InfSpeedXVar,
	                     ConvectiveFast_S->InfSpeedYVar);

	auto BottomNodes = GetTreeBottomNodes();
	if ( !BottomNodes ) return -1;

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
				TObj &obj = **llobj;
				dr_local = obj - TVec(bnode.x, bnode.y);
				obj.v += TVec(Teilor1, Teilor2) + infspeed + SpeedSum(bnode, obj) +
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
				obj.v += TVec(Teilor1, Teilor2) + infspeed + SpeedSum(bnode, obj) +
				         TVec(TVec(Teilor3,  Teilor4)*dr_local, 
				              TVec(Teilor4, -Teilor3)*dr_local);
			}
		}
	}

	return 0;
}

int CalcBoundaryConvective()
{
	auto vlist = ConvectiveFast_S->VortexList;
	//TList<TObj> *hlist = ConvectiveFast_S->HeatList;

	if (!vlist) { return -1; }

	const_for(vlist, lobj)
	{
		lobj->v += BoundaryConvective(*lobj)*C_1_2PI;
	}

	return 0;
}

namespace {
TVec BoundaryConvective(const TVec &p)
{
	TVec dr, res(0, 0);
	auto alist = ConvectiveFast_S->Body->AttachList;
	double rotspeed = ConvectiveFast_S->Body->RotationVVar;

	if (rotspeed)
	const_for(alist, latt)
	{
		dr = p - *latt;
		res += (dr*latt->q + rotl(dr)*latt->g) * (rotspeed/( dr.abs2() + ConvectiveFast_Eps ));
	}
	return res;
}}

int CalcCirculationFast()
{
	//if (!BodyMatrixOK)
		//FillMatrix();

	FillRightCol();
	SolveMatrix();

	auto NakedBodyList = ConvectiveFast_S->Body->List->begin();
	for (int i=0; i< N; i++)
	{
		NakedBodyList[i].g = Solution[i];
	}

	return 0;
}


namespace {
//inline
double ObjectInfluence(TObj &obj, TObj &seg1, TObj &seg2, double eps)
{
	TVec res;
	fortobjectinfluence_(&obj.rx, &obj.ry, &seg1.rx, &seg1.ry, &seg2.rx, &seg2.ry, &res.rx, &res.ry, &eps);
	TVec dl=seg2-seg1;
	return -(rotl(res)*dl)/dl.abs()*C_1_2PI;
	//FIXME kill fortran
}}

namespace {
//inline
double NodeInfluence(TNode &Node, TObj &seg1, TObj &seg2, double eps)
{
	TVec res(0, 0), tmp;

	const_for(Node.NearNodes, llnnode)
	{
		auto vlist = (**llnnode).VortexLList;
		if ( !vlist ) { continue; }

		const_for (vlist, llobj)
		{
			TObj &obj = **llobj;
			fortobjectinfluence_(&obj.rx, &obj.ry, &seg1.rx, &seg1.ry, &seg2.rx, &seg2.ry, &tmp.rx, &tmp.ry, &eps);
			res+= tmp*obj.g;
		}
	}

	const_for(Node.FarNodes, llfnode)
	{
		TNode &fnode = **llfnode;

		fortobjectinfluence_(&fnode.CMp.rx, &fnode.CMp.ry, &seg1.rx, &seg1.ry, &seg2.rx, &seg2.ry, &tmp.rx, &tmp.ry, &eps);
		res+= tmp*fnode.CMp.g;
		fortobjectinfluence_(&fnode.CMm.rx, &fnode.CMm.ry, &seg1.rx, &seg1.ry, &seg2.rx, &seg2.ry, &tmp.rx, &tmp.ry, &eps);
		res+= tmp*fnode.CMm.g;
	}

	TVec dl=seg2-seg1;
	return -(rotl(res)*dl)/dl.abs()*C_1_2PI;
}}

namespace {
//inline
double AttachInfluence(TObj &seg1, TObj &seg2, const TAtt &center, double eps)
{
	auto alist = ConvectiveFast_S->Body->AttachList;
	if (!alist->size()) { return 0; }

	TVec res(0, 0), tmp;

	const_for(alist, latt)
	{
		if (latt == &center) continue;
		fortobjectinfluence_(&latt->rx, &latt->ry, &seg1.rx, &seg1.ry, &seg2.rx, &seg2.ry, &tmp.rx, &tmp.ry, &eps);
		res+= tmp*latt->g - rotl(tmp)*latt->q;
	}
	TVec dl=seg2-seg1;
	return -(rotl(res)*dl)/dl.abs()*C_1_2PI - center.q*0.5; //FIXME sign
	//FIXME kill fortran
}}


int FillMatrix()
{
	int i, j;
	int imax = N-1;
	//BodyMatrix[N*i+j]
	//RightCol[i]

	auto BVort = ConvectiveFast_S->Body->List->begin();
	auto FirstBVort = ConvectiveFast_S->Body->List->begin();
	auto LastBVort = ConvectiveFast_S->Body->List->end();
	for ( ; BVort<LastBVort; BVort++)
	{
		//temporarily vort->g stores eps info.
		TVec dr1 = *BVort - ( (BVort>FirstBVort)?*(BVort-1):*(LastBVort-1) );
		TVec dr2 = *BVort - ( (BVort<(LastBVort-1))?*(BVort+1):*FirstBVort );
		BVort->g = (dr1.abs() + dr2.abs())*0.25;
	}

	auto NakedBodyList = ConvectiveFast_S->Body->List->begin();
	for (i=0; i<imax; i++)
	{
		double *RowI = BodyMatrix + N*i;
		for (j=0; j<N; j++)
		{
			RowI[j] = ObjectInfluence(NakedBodyList[j], NakedBodyList[i], NakedBodyList[i+1], NakedBodyList[j].g);
		}
	}
		double *RowI = BodyMatrix + N*imax;
		for (j=0; j<N; j++)
		{
			RowI[j] = 1;
		}
//			RowI[0] = 1; //Joukowski condition

	BodyMatrixOK = true;
	return 0;
}

int FillRightCol()
{
	auto NakedBodyList = ConvectiveFast_S->Body->List->begin();
	auto alist = ConvectiveFast_S->Body->AttachList;
	int imax = N-1;

	#pragma omp parallel for
	for (int i=0; i<imax; i++)
	{
		TNode* Node = FindNode(NakedBodyList[i]);
		if (!Node) { continue; }

		TVec SegDl = NakedBodyList[i+1] - NakedBodyList[i];

		RightCol[i] = -ConvectiveFast_S->InfSpeedYVar*SegDl.rx + ConvectiveFast_S->InfSpeedXVar*SegDl.ry -
		               NodeInfluence(*Node, NakedBodyList[i], NakedBodyList[i+1], ConvectiveFast_Eps) -
		               ((!ConvectiveFast_S->Body->RotationVVar)?0:AttachInfluence(NakedBodyList[i], NakedBodyList[i+1], alist->at(i), ConvectiveFast_Eps)*
		               ConvectiveFast_S->Body->RotationVVar);
	}

	double tmpgsum =0;
	if (ConvectiveFast_S->Body->RotationVVar)
	{
		auto lAtt = alist->begin();
		auto lLastAtt = alist->end();
		for ( ; lAtt<lLastAtt; lAtt++ )
		{ tmpgsum += lAtt->g; }
	}

	RightCol[imax] = -ConvectiveFast_S->gsum() - ConvectiveFast_S->Body->RotationVVar*tmpgsum;

	return 0;
}

int SolveMatrix()
{
	if (!InverseMatrixOK)
	{
		cerr << "Inverse!\n";
		return -1; //Inverse
	}

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

	return 0;
}

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

