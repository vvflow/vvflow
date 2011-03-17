#include "convectivefast.h"
#include "stdio.h"
#include "stdlib.h"
#include "iostream"

using namespace std;

/********************* HEADER ****************************/

namespace {

Space *ConvectiveFast_S;
double ConvectiveFast_Eps;

inline void BioSavar(TVortex &vort, double px, double py, double &resx, double &resy);
void SpeedSum(TNode &Node, double px, double py, double &resx, double &resy);

int N; // BodySize;
double *BodyMatrix;
double *InverseMatrix;
double *RightCol;
double *Solution;

bool BodyMatrixOK;
bool InverseMatrixOK; 

double ObjectInfluence(TObject &obj, TObject &seg1, TObject &seg2);
int LoadMatrix(double *matrix, const char* filename);


} //end of namespace

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

/*int SpeedSum(double px, double py, double &resx, double &resy)
{
	here shoult be next functions:

	FindNode(px, py);
	SpeedSum(List, px, py, 1, 0, resx, resy);
	Far nodes cycle

	int fnlsize; //Far nodes list size
	TNode **lFNode = (TNode**)Node->FarNodes->Elements; //link to FarNode link
	TNode *FNode;
	fnlsize = Node->FarNodes->size;

	for ( i=0; i<fnlsize; i++ )
	{
		FNode = *lFNode;

		double CMresx, CMresy;
		BioSavar(&FNode->CMp, px, py, CMresx, CMresy);
		resx+= CMresx; resy+= CMresy;
		BioSavar(&FNode->CMm, px, py, CMresx, CMresy);
		resx+= CMresx; resy+= CMresy;

		lFNode++;
	}
	return 0;
}*/

namespace {
inline
void BioSavar(TVortex &Vort, double px, double py, double &resx, double &resy)
{
	double drx, dry;
	double multiplier;

	drx = px - Vort.rx;
	dry = py - Vort.ry;
	//drabs2 = drx*drx + dry*dry;
	#define drabs2 drx*drx + dry*dry
	multiplier = Vort.g / ( drabs2 + ConvectiveFast_Eps ) * C_1_2PI;
	#undef drabs2
	resx = -dry * multiplier;
	resy =  drx * multiplier;
}}


namespace {
void SpeedSum(TNode &Node, double px, double py, double &resx, double &resy)
{
	double drx, dry;
	double multiplier;

	resx = resy = 0;
	TNode** lNode = Node.NearNodes->First;
	TNode** &LastNode = Node.NearNodes->Last;
	for ( ; lNode<LastNode; lNode++ )
	{
		//TNode &NNode = **lNode;
		TList<TObject*> *vList = (**lNode).VortexLList;
		if ( !vList ) { continue; }
		
		TVortex** lVort = vList->First;
		TVortex** LastVort = vList->Last;
		for ( ; lVort<LastVort; lVort++ )
		{
			TVortex &Vort = **lVort;
			drx = px - Vort.rx;
			dry = py - Vort.ry;
			//drabs2 = drx*drx + dry*dry;
			#define drabs2 drx*drx + dry*dry
			multiplier = Vort.g / ( drabs2 + ConvectiveFast_Eps ); // 1/2PI is in flowmove
			#undef drabs2
			resx -= dry * multiplier;
			resy += drx * multiplier;
		}
	}

	resx *= C_1_2PI;
	resy *= C_1_2PI;
}}

int CalcConvectiveFast()
{
	double SpeedSumResX, SpeedSumResY;
	double Teilor1, Teilor2, Teilor3, Teilor4;

	//initialization of InfSpeed & Rotation
	double InfX = ConvectiveFast_S->InfSpeedXVar; 
	double InfY = ConvectiveFast_S->InfSpeedYVar;
	double RotationG = ConvectiveFast_S->RotationVVar * C_2PI;

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
		double multiplier;
		#define SpeedSumCircle(List) 														\
		if (List) 																			\
		{ 																					\
			TObject **lObj = List->First; 													\
			TObject **&LastObj = List->Last; 												\
			for ( ; lObj<LastObj; lObj++ ) 													\
			{																				\
				TObject &Obj = **lObj; 														\
				multiplier = RotationG ? RotationG/(Obj.rx*Obj.rx + Obj.ry*Obj.ry + ConvectiveFast_Eps) : 0; \
				LocaldRx = Obj.rx - BNode.x; 												\
				LocaldRy = Obj.ry - BNode.y; 												\
				SpeedSum(BNode, Obj.rx, Obj.ry, SpeedSumResX, SpeedSumResY); 				\
				Obj.vx += Teilor1 + Teilor3*LocaldRx + Teilor4*LocaldRy + 					\
							SpeedSumResX + InfX - Obj.ry*multiplier; 						\
				Obj.vy += Teilor2 + Teilor4*LocaldRx - Teilor3*LocaldRy + 					\
							SpeedSumResY + InfY + Obj.rx*multiplier; 						\
			} 																				\
		}

		SpeedSumCircle(BNode.VortexLList);
		SpeedSumCircle(BNode.HeatLList);
	}

	return 0;
}


int CalcCirculationFast()
{
	if (!BodyMatrixOK)
		FillMatrix();

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
double ObjectInfluence(TObject &obj, TObject &seg1, TObject &seg2)
{
	return 0;
}}


int FillMatrix()
{
	int i, j;
	int imax = N-1;
	//BodyMatrix[N*i+j]
	//RightCol[i]
	TObject *NakedBodyList = ConvectiveFast_S->Body->List->First;
	for (i=0; i<imax; i++)
	{
		double *RowI = BodyMatrix + N*i;
		for (j=0; j<N; j++)
		{
			RowI[j] = ObjectInfluence(NakedBodyList[j], NakedBodyList[i], NakedBodyList[i+1]);
		}
	}
		double *RowI = BodyMatrix + N*imax;
		for (j=0; j<N; j++)
		{
			RowI[j] = 1;
		}

	BodyMatrixOK = true;
	return 0;
}

int FillRightCol()
{
	return 0;
}

int SolveMatrix()
{
	if (!InverseMatrixOK)
		; //Inverse

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

int LoadBodyMatrix(const char* filename)
{ 
	BodyMatrixOK = (LoadMatrix(BodyMatrix, filename) == N*N);
	return BodyMatrixOK;
}

int LoadInverseMatrix(const char* filename)
{
	InverseMatrixOK = (LoadMatrix(InverseMatrix, filename) == N*N);
	return InverseMatrixOK;
}

int LoadMatrix(double *matrix, const char* filename)
{
	if ( !matrix ) return -1;
	FILE *fin;

	fin = fopen(filename, "r");
	if (!fin) { cerr << "No file called " << filename << endl; return -1; } 
	double *dst = matrix;
	while ( fscanf(fin, "%lf", dst)==1 )
	{
		dst++;
	}
	fclose(fin);
	return (dst - matrix);
	//FIXME may work unproperly
}
