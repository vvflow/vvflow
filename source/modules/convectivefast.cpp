#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "convectivefast.h"

using namespace std;

/********************* HEADER ****************************/

namespace {

Space *ConvectiveFast_S;
double ConvectiveFast_Eps;

inline
Vector BioSavar(const TObject &obj, const Vector &p);
Vector SpeedSum(const TNode &Node, const Vector &p);

int N; // BodySize;
double *BodyMatrix;
double *InverseMatrix;
double *RightCol;
double *Solution;

bool BodyMatrixOK;
bool InverseMatrixOK; 

double ObjectInfluence(TObject &obj, TObject &seg1, TObject &seg2, double eps);
double NodeInfluence(TNode &Node, TObject &seg1, TObject &seg2, double eps);
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

	N = sS->Body->List->size;
	BodyMatrix = (double*)malloc(N*N*sizeof(double));
	InverseMatrix = (double*)malloc(N*N*sizeof(double));
	RightCol = (double*)malloc(N*sizeof(double));
	Solution = (double*)malloc(N*sizeof(double));
	BodyMatrixOK = InverseMatrixOK = false;
	return 0;
}

Vector SpeedSumFast(const Vector p)
{
	Vector res(0, 0);
	TNode* Node = FindNode(p.rx, p.ry);
	if (!Node) return res;
	res = SpeedSum(*Node, p);

	TNode** lFNode = Node->FarNodes->First;
	TNode** LastFNode = Node->FarNodes->Last;
	for ( ; lFNode<LastFNode; lFNode++ )
	{
		TNode &FNode = **lFNode;

		res+= BioSavar(FNode.CMp, p) * C_1_2PI;
		res+= BioSavar(FNode.CMm, p) * C_1_2PI;
	}
	return res;
}

namespace {
inline
Vector BioSavar(const TObject &obj, const Vector &p)
{
	Vector dr = p - obj;
	return rotl(dr)*(obj.g / (dr.abs2() + ConvectiveFast_Eps) );
}}


namespace {
Vector SpeedSum(const TNode &Node, const Vector p)
{
	Vector dr, res(0, 0);

	TNode** lNode = Node.NearNodes->First;
	TNode** &LastNode = Node.NearNodes->Last;
	for ( ; lNode<LastNode; lNode++ )
	{
		//TNode &NNode = **lNode;
		TList<TObject*> *vList = (**lNode).VortexLList;
		if ( !vList ) { continue; }
		
		TObject** lVort = vList->First;
		TObject** LastVort = vList->Last;
		for ( ; lVort<LastVort; lVort++ )
		{
			res+= BioSavar(**lVort, p); 
		}
	}

	res *= C_1_2PI;
	return res;
}}

int CalcConvectiveFast()
{
	double SpeedSumResX, SpeedSumResY;
	double Teilor1, Teilor2, Teilor3, Teilor4;

	//initialization of InfSpeed & Rotation
	double InfX = ConvectiveFast_S->InfSpeedXVar; 
	double InfY = ConvectiveFast_S->InfSpeedYVar;
	//double RotationG = ConvectiveFast_S->RotationVVar * C_2PI;

	TList<TNode*> *BottomNodes = GetTreeBottomNodes();
	if ( !BottomNodes ) return -1;

	TNode** lBNode = BottomNodes->First;
	TNode** LastBNode = BottomNodes->Last;
	for ( ; lBNode<LastBNode; lBNode++ )
	{
		TNode &BNode = **lBNode;

		double DistPx, DistPy; //Distance between current node center and positive center of mass of far node 
		double DistMx, DistMy;
		double FuncP1, FuncM1; //Extremely complicated useless variables
		double FuncP2, FuncM2;

		Teilor1 = Teilor2 = Teilor3 = Teilor4 = 0;

		TNode** lFNode = BNode.FarNodes->First;
		TNode** LastFNode = BNode.FarNodes->Last;
		for ( ; lFNode<LastFNode; lFNode++ )
		{
			TNode &FarNode = **lFNode;
			DistPx = BNode.x - FarNode.CMp.rx;
			DistPy = BNode.y - FarNode.CMp.ry;
			DistMx = BNode.x - FarNode.CMm.rx;
			DistMy = BNode.y - FarNode.CMm.ry;
			
			double _1_DistPabs = 1/(DistPx*DistPx + DistPy*DistPy);
			double _1_DistMabs = 1/(DistMx*DistMx + DistMy*DistMy);
			FuncP1 = FarNode.CMp.g * _1_DistPabs; //Extremely complicated useless variables
			FuncM1 = FarNode.CMm.g * _1_DistMabs;
			FuncP2 = FuncP1 * _1_DistPabs;
			FuncM2 = FuncM1 * _1_DistMabs;
			
			Teilor1 -= (FuncP1*DistPy + FuncM1*DistMy);
			Teilor2 += (FuncP1*DistPx + FuncM1*DistMx);
			Teilor3 += (FuncP2*DistPy*DistPx + FuncM2*DistMy*DistMx);
			Teilor4 += (FuncP2 * (DistPy*DistPy - DistPx*DistPx) + FuncM2 * (DistMy*DistMy - DistMx*DistMx));
		}

		Teilor1 *= C_1_2PI;
		Teilor2 *= C_1_2PI;
		Teilor3 *= C_1_PI;
		Teilor4 *= C_1_2PI;

		double LocaldRx, LocaldRy;
		//double multiplier;
		#define SpeedSumCircle(List) 														\
		if (List) 																			\
		{ 																					\
			TObject **lObj = List->First; 													\
			TObject **&LastObj = List->Last; 												\
			for ( ; lObj<LastObj; lObj++ ) 													\
			{																				\
				TObject &Obj = **lObj; 														\
				LocaldRx = Obj.rx - BNode.x; 												\
				LocaldRy = Obj.ry - BNode.y; 												\
				SpeedSum(BNode, Obj.rx, Obj.ry, SpeedSumResX, SpeedSumResY); 				\
				Obj.vx += Teilor1 + Teilor3*LocaldRx + Teilor4*LocaldRy + 					\
							SpeedSumResX + InfX; 						\
				Obj.vy += Teilor2 + Teilor4*LocaldRx - Teilor3*LocaldRy + 					\
							SpeedSumResY + InfY; 						\
			} 																				\
		}

		SpeedSumCircle(BNode.VortexLList);
		SpeedSumCircle(BNode.HeatLList);
	}

	return 0;
}

int CalcBoundaryConvective()
{
	double drx, dry, multiplier, resx, resy;

	TList<TObject> *vlist = ConvectiveFast_S->VortexList;
	TList<TObject> *blist = ConvectiveFast_S->Body->List;
	//TList<TObject> *hlist = ConvectiveFast_S->HeatList;

	if (vlist)
	{
		TObject *Obj = vlist->First;
		TObject *&LastObj = vlist->Last;
		for( ; Obj<LastObj; Obj++ )
		{
			resx = resy = 0;

			TObject *BVort = blist->First;
			TObject *&LastBVort = blist->Last;
			for ( ; BVort<LastBVort; BVort++ )
			{
				drx = Obj->rx - BVort->rx;
				dry = Obj->ry - BVort->ry;
				multiplier = BVort->g / ( drx*drx+dry*dry + ConvectiveFast_Eps );
				resx -= dry * multiplier;
				resy += drx * multiplier;
			}

			Obj->vx += resx*C_1_2PI;
			Obj->vy += resy*C_1_2PI;
		}
	}

	return 0;
}

int CalcCirculationFast()
{
	//if (!BodyMatrixOK)
		//FillMatrix();

	FillRightCol();
	SolveMatrix();

	TObject *NakedBodyList = ConvectiveFast_S->Body->List->First;
	for (int i=0; i< N; i++)
	{
		NakedBodyList[i].g = Solution[i];
	}

	return 0;
}


namespace {
inline
double ObjectInfluence(TObject &obj, TObject &seg1, TObject &seg2, double eps)
{
	double resx, resy;
	fortobjectinfluence_(&obj.rx, &obj.ry, &seg1.rx, &seg1.ry, &seg2.rx, &seg2.ry, &resx, &resy, &eps);
	double dx=seg2.rx-seg1.rx;
	double dy=seg2.ry-seg1.ry;
	return (resy*dx - resx*dy)/sqrt(dx*dx+dy*dy)*C_1_2PI;
	//FIXME kill fortran
}}

namespace {
inline
double NodeInfluence(TNode &Node, TObject &seg1, TObject &seg2, double eps)
{
	double resx=0, resy=0;
	double tmpx, tmpy;

	TNode** lNode = Node.NearNodes->First;
	TNode** &LastNode = Node.NearNodes->Last;
	for ( ; lNode<LastNode; lNode++ )
	{
		TList<TObject*> *vList = (**lNode).VortexLList;
		if ( !vList ) { continue; }

		TObject** lVort = vList->First;
		TObject** LastVort = vList->Last;
		for ( ; lVort<LastVort; lVort++ )
		{
			TObject &Vort = **lVort;
			fortobjectinfluence_(&Vort.rx, &Vort.ry, &seg1.rx, &seg1.ry, &seg2.rx, &seg2.ry, &tmpx, &tmpy, &eps);
			resx+= tmpx*Vort.g; resy+= tmpy*Vort.g;
		}
	}

	TNode** lFNode = Node.FarNodes->First;
	TNode** LastFNode = Node.FarNodes->Last;
	for ( ; lFNode<LastFNode; lFNode++ )
	{
		TNode &FNode = **lFNode;

		fortobjectinfluence_(&FNode.CMp.rx, &FNode.CMp.ry, &seg1.rx, &seg1.ry, &seg2.rx, &seg2.ry, &tmpx, &tmpy, &eps);
		resx+= tmpx*FNode.CMp.g; resy+= tmpy*FNode.CMp.g;
		fortobjectinfluence_(&FNode.CMm.rx, &FNode.CMm.ry, &seg1.rx, &seg1.ry, &seg2.rx, &seg2.ry, &tmpx, &tmpy, &eps);
		resx+= tmpx*FNode.CMm.g; resy+= tmpy*FNode.CMm.g;
	}

	double dx=seg2.rx-seg1.rx;
	double dy=seg2.ry-seg1.ry;
	return (resy*dx - resx*dy)/sqrt(dx*dx+dy*dy)*C_1_2PI;
}}


int FillMatrix()
{
	int i, j;
	int imax = N-1;
	//BodyMatrix[N*i+j]
	//RightCol[i]

	TObject *BVort = ConvectiveFast_S->Body->List->First;
	TObject *&FirstBVort = ConvectiveFast_S->Body->List->First;
	TObject *&LastBVort = ConvectiveFast_S->Body->List->Last;
	for ( ; BVort<LastBVort; BVort++)
	{
		//temporarily vort->g stores eps info.
		double dx1 = BVort->rx - ( (BVort>FirstBVort)?(BVort-1)->rx:(LastBVort-1)->rx );
		double dy1 = BVort->ry - ( (BVort>FirstBVort)?(BVort-1)->ry:(LastBVort-1)->ry );
		double dx2 = BVort->rx - ( (BVort<(LastBVort-1))?(BVort+1)->rx:FirstBVort->rx );
		double dy2 = BVort->ry - ( (BVort<(LastBVort-1))?(BVort+1)->ry:FirstBVort->ry );
		BVort->g = (sqrt(dx1*dx1+dy1*dy1) + sqrt(dx2*dx2+dy2*dy2))*0.25;
	}

	TObject *NakedBodyList = ConvectiveFast_S->Body->List->First;
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
	TObject *NakedBodyList = ConvectiveFast_S->Body->List->First;
	int imax = N-1;
	for (int i=0; i<imax; i++)
	{
		TNode* Node = FindNode(NakedBodyList[i].rx, NakedBodyList[i].ry);
		if (!Node) { cerr << "fail" << endl; return -1;}

		double SegDx = NakedBodyList[i+1].rx - NakedBodyList[i].rx;
		double SegDy = NakedBodyList[i+1].ry - NakedBodyList[i].ry;

		RightCol[i] = -ConvectiveFast_S->InfSpeedYVar*SegDx + ConvectiveFast_S->InfSpeedXVar*SegDy -
			NodeInfluence(*Node, NakedBodyList[i], NakedBodyList[i+1], ConvectiveFast_Eps);
	}

	RightCol[imax] = -ConvectiveFast_S->gsum();

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
