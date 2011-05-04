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

Vector BoundaryConvective(const Vector &p);

int N; // BodySize;
double *BodyMatrix;
double *InverseMatrix;
double *RightCol;
double *Solution;

bool BodyMatrixOK;
bool InverseMatrixOK; 

double ObjectInfluence(TObject &obj, TObject &seg1, TObject &seg2, double eps);
double NodeInfluence(TNode &Node, TObject &seg1, TObject &seg2, double eps);
double AttachInfluence(TObject &seg1, TObject &seg2, const TAttach &center, double eps);
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
Vector SpeedSum(const TNode &Node, const Vector &p)
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

		Vector DistP, DistM; //Distance between current node center and positive center of mass of far node 
		double FuncP1, FuncM1; //Extremely complicated useless variables
		double FuncP2, FuncM2;

		Teilor1 = Teilor2 = Teilor3 = Teilor4 = 0;

		TNode** lFNode = BNode.FarNodes->First;
		TNode** LastFNode = BNode.FarNodes->Last;
		for ( ; lFNode<LastFNode; lFNode++ )
		{
			TNode &FarNode = **lFNode;
			DistP = Vector(BNode.x, BNode.y) - FarNode.CMp;
			DistM = Vector(BNode.x, BNode.y) - FarNode.CMm;
			
			double _1_DistPabs = 1/DistP.abs2();
			double _1_DistMabs = 1/DistM.abs2();
			FuncP1 = FarNode.CMp.g * _1_DistPabs; //Extremely complicated useless variables
			FuncM1 = FarNode.CMm.g * _1_DistMabs;
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

		Vector dr_local;
		
		if (BNode.VortexLList)
		{
			TObject **lObj = BNode.VortexLList->First;
			TObject **&LastObj = BNode.VortexLList->Last;
			for ( ; lObj<LastObj; lObj++ )
			{
				TObject &Obj = **lObj;
				dr_local = Obj - Vector(BNode.x, BNode.y);
				Obj.v += Vector(Teilor1, Teilor2) + Vector(InfX, InfY) + SpeedSum(BNode, Obj) +
							Vector(Vector(Teilor3, Teilor4)*dr_local, 
									Vector(Teilor4, -Teilor3)*dr_local);
			}
		}

		if (BNode.HeatLList)
		{
			TObject **lObj = BNode.HeatLList->First;
			TObject **&LastObj = BNode.HeatLList->Last;
			for ( ; lObj<LastObj; lObj++ )
			{
				TObject &Obj = **lObj;
				dr_local = Obj - Vector(BNode.x, BNode.y);
				Obj.v += Vector(Teilor1, Teilor2) + Vector(InfX, InfY) + SpeedSum(BNode, Obj) +
							Vector(Vector(Teilor3, Teilor4)*dr_local, 
									Vector(Teilor4, -Teilor3)*dr_local);
			}
		}
	}

	return 0;
}

int CalcBoundaryConvective()
{
	TList<TObject> *vlist = ConvectiveFast_S->VortexList;
	//TList<TObject> *hlist = ConvectiveFast_S->HeatList;

	if (!vlist) { return -1; }

	TObject *Obj = vlist->First;
	TObject *&LastObj = vlist->Last;
	for( ; Obj<LastObj; Obj++ )
	{
		Obj->v += BoundaryConvective(*Obj)*C_1_2PI;
	}

	return 0;
}

namespace {
Vector BoundaryConvective(const Vector &p)
{
	Vector dr, res(0, 0);
	TList<TAttach> *alist = ConvectiveFast_S->Body->AttachList;

	TAttach *Att = alist->First;
	TAttach *&LastAtt = alist->Last;
	for ( ; Att<LastAtt; Att++ )
	{
		dr = p - *Att;
		res += (dr*Att->q + rotl(dr)*Att->g) * (ConvectiveFast_S->Body->RotationVVar/( dr.abs2() + ConvectiveFast_Eps ));
		//FIXME signs
	}
	//if (res.rx!=res.rx) { cout << p << endl; }
	return res;
}}

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
//inline
double ObjectInfluence(TObject &obj, TObject &seg1, TObject &seg2, double eps)
{
	Vector res;
	fortobjectinfluence_(&obj.rx, &obj.ry, &seg1.rx, &seg1.ry, &seg2.rx, &seg2.ry, &res.rx, &res.ry, &eps);
	Vector dl=seg2-seg1;
	return -(rotl(res)*dl)/dl.abs()*C_1_2PI;
	//FIXME kill fortran
}}

namespace {
//inline
double NodeInfluence(TNode &Node, TObject &seg1, TObject &seg2, double eps)
{
	Vector res(0, 0), tmp;

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
			fortobjectinfluence_(&Vort.rx, &Vort.ry, &seg1.rx, &seg1.ry, &seg2.rx, &seg2.ry, &tmp.rx, &tmp.ry, &eps);
			res+= tmp*Vort.g;
		}
	}

	TNode** lFNode = Node.FarNodes->First;
	TNode** LastFNode = Node.FarNodes->Last;
	for ( ; lFNode<LastFNode; lFNode++ )
	{
		TNode &FNode = **lFNode;

		fortobjectinfluence_(&FNode.CMp.rx, &FNode.CMp.ry, &seg1.rx, &seg1.ry, &seg2.rx, &seg2.ry, &tmp.rx, &tmp.ry, &eps);
		res+= tmp*FNode.CMp.g;
		fortobjectinfluence_(&FNode.CMm.rx, &FNode.CMm.ry, &seg1.rx, &seg1.ry, &seg2.rx, &seg2.ry, &tmp.rx, &tmp.ry, &eps);
		res+= tmp*FNode.CMm.g;
	}

	Vector dl=seg2-seg1;
	return -(rotl(res)*dl)/dl.abs()*C_1_2PI;
}}

namespace {
//inline
double AttachInfluence(TObject &seg1, TObject &seg2, const TAttach &center, double eps)
{
	TList<TAttach> *alist = ConvectiveFast_S->Body->AttachList;
	if (!alist->size) { return 0; }

	double resx=0, resy=0;
	double tmpx, tmpy;

	TAttach* lAtt = alist->First;
	TAttach* &LastAtt = alist->Last;
	for ( ; lAtt<LastAtt; lAtt++ )
	{
		if (lAtt == &center) continue;
		fortobjectinfluence_(&lAtt->rx, &lAtt->ry, &seg1.rx, &seg1.ry, &seg2.rx, &seg2.ry, &tmpx, &tmpy, &eps);
		resx+= tmpx*lAtt->g+tmpy*lAtt->q; resy+= tmpy*lAtt->g-tmpx*lAtt->q;
	}
	double dx=seg2.rx-seg1.rx;
	double dy=seg2.ry-seg1.ry;
	return (resy*dx - resx*dy)/sqrt(dx*dx+dy*dy)*C_1_2PI - center.q*0.5; //FIXME sign
	//FIXME kill fortran
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
		Vector dr1 = *BVort - ( (BVort>FirstBVort)?*(BVort-1):*(LastBVort-1) );
		Vector dr2 = *BVort - ( (BVort<(LastBVort-1))?*(BVort+1):*FirstBVort );
		BVort->g = (dr1.abs() + dr2.abs())*0.25;
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
	TList<TAttach> *alist = ConvectiveFast_S->Body->AttachList;
	int imax = N-1;
	for (int i=0; i<imax; i++)
	{
		TNode* Node = FindNode(NakedBodyList[i].rx, NakedBodyList[i].ry);
		if (!Node) { cerr << "fail" << endl; return -1;}

		Vector SegDl = NakedBodyList[i+1] - NakedBodyList[i];

		RightCol[i] = -ConvectiveFast_S->InfSpeedYVar*SegDl.rx + ConvectiveFast_S->InfSpeedXVar*SegDl.ry -
		               NodeInfluence(*Node, NakedBodyList[i], NakedBodyList[i+1], ConvectiveFast_Eps) -
		               ((!ConvectiveFast_S->Body->RotationVVar)?0:AttachInfluence(NakedBodyList[i], NakedBodyList[i+1], alist->item(i), ConvectiveFast_Eps)*
		               ConvectiveFast_S->Body->RotationVVar);
	}

	double tmpgsum =0;
	if (ConvectiveFast_S->Body->RotationVVar)
	{
		TAttach *lAtt = alist->First;
		TAttach *lLastAtt = alist->Last;
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

