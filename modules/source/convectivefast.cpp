#include "convective.h"
#include "iostream"

using namespace std;

/********************* HEADER ****************************/

namespace {

Space *ConvectiveFast_S;
double ConvectiveFast_Eps;

inline void BioSavar(TVortex &vort, double px, double py, double &resx, double &resy);
inline void BioSavarIm(TVortex &vort, double px, double py, double &resx, double &resy);
void SpeedSum(TNode &Node, double px, double py, double &resx, double &resy);

} //end of namespace

/********************* SOURCE *****************************/

int InitConvectiveFast(Space *sS, double sEps)
{
	ConvectiveFast_S = sS;
	ConvectiveFast_Eps = sEps;
	return 0;
}

/*int SpeedSum(TList *List, double px, double py, double &resx, double &resy)
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
inline
void BioSavarIm(TVortex &Vort, double px, double py, double &resx, double &resy)
{
	double drx, dry;
	double multiplier;
	double vortrabs2 = (Vort.rx*Vort.rx + Vort.ry*Vort.ry);
	if (!vortrabs2) { resx = resy = 0; return; }
	double vort1rabs2 = 1/vortrabs2;

	drx = px - Vort.rx * vort1rabs2;
	dry = py - Vort.ry * vort1rabs2;
	//drabs2 = drx*drx + dry*dry;
	#define drabs2 drx*drx + dry*dry
	multiplier = Vort.g / ( drabs2 + ConvectiveFast_Eps ) * C_1_2PI;
	#undef drabs2
	resx =  dry * multiplier;
	resy = -drx * multiplier;
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
	if ( !ConvectiveFast_S->BodyList ) return -1;

	double TwoInfX = 2 * ConvectiveFast_S->InfSpeedXVar; 
	double TwoInfY = 2 * ConvectiveFast_S->InfSpeedYVar;
	double RotationG = ConvectiveFast_S->RotationVVar * C_2PI;

	double dfi = C_2PI/ConvectiveFast_S->BodyList->size;
	double SqrOfHalfDfi = dfi*dfi*0.25;
	double CirculationAdditionDueToRotation = RotationG/ConvectiveFast_S->BodyList->size;


	TList<TNode*> *BottomNodes = GetTreeBottomNodes();
	if ( !BottomNodes ) return -1;

	TNode **lBNode = BottomNodes->First; //Bottom Node
	TNode **&LastBNode = BottomNodes->Last;
	for ( ; lBNode<LastBNode; lBNode++ )
	{
		TNode &BNode = **lBNode;
		if ( !BNode.BodyLList ) { continue; }

		TVortex **lBVort = BNode.BodyLList->First; //Body Vortex
		TVortex **&LastBVort = BNode.BodyLList->Last;
		for ( ; lBVort<LastBVort; lBVort++ )
		{
			TVortex &BVort = **lBVort;
			double &BVortX = BVort.rx; //i don't know if it's effective or not
			double &BVortY = BVort.ry;
			BVort.g = 0;

			TNode **lNNode = BNode.NearNodes->First; //Near Node
			TNode **&LastNNode = BNode.NearNodes->Last;
			for ( ; lNNode<LastNNode; lNNode++ )
			{
				TNode &NNode = **lNNode;

				if ( !NNode.VortexLList ) { continue; }
				//vortrex atan circle
				//weak place

				double AtanSum=0;
				TVortex **lNVort = NNode.VortexLList->First; //Near Vortex
				TVortex **&LastNVort = NNode.VortexLList->Last;
				for ( ; lNVort<LastNVort; lNVort++ )
				{
					TVortex &NVort = **lNVort;
					double ScalarMult = BVortX*NVort.rx + BVortY*NVort.ry;
					double ObjRSqr = NVort.rx*NVort.rx + NVort.ry*NVort.ry;
					double NumDifferx = NVort.rx - BVortX;
					double NumDiffery = NVort.ry - BVortY;
					double NumDifferSqr = NumDifferx*NumDifferx + NumDiffery*NumDiffery;
					double Num1DifferSqr = 1./NumDifferSqr;
					double Obj1Rsqr = 1./ObjRSqr;
					int flagis = 1;
					
					if ( ObjRSqr < 1 )  flagis = -1;
					if ( NumDifferSqr > SqrOfHalfDfi*120)
					{
						AtanSum += -flagis*NVort.g * (ObjRSqr-1)*Num1DifferSqr;
					}
					else
					{
						AtanSum += flagis * ( atan2( ((1-ScalarMult)*dfi), (1 + ObjRSqr - 2*ScalarMult - SqrOfHalfDfi) )
								-atan2( ((1-ScalarMult*Obj1Rsqr)*dfi), (1 + (1 - 2*ScalarMult)*Obj1Rsqr - SqrOfHalfDfi) )
								) * NVort.g / dfi; //AAAAARRRRRGGGGGHHHHH!!!!
					}

					/*BioSavar(obj, BVortX, BVortY, SpeedSumResX, SpeedSumResY);
					SpeedSumX += SpeedSumResX; SpeedSumY += SpeedSumResY;
					BioSavarIm(obj, BVortX, BVortY, SpeedSumResX, SpeedSumResY);
					SpeedSumX += SpeedSumResX; SpeedSumY += SpeedSumResY;*/ //useless, instead of atan
				}
				BVort.g += AtanSum*C_1_2PI; //*dfi at the end
			}

			double SpeedSumX, SpeedSumY;
			double SpeedSumResX, SpeedSumResY;
			SpeedSumX = SpeedSumY = SpeedSumResX = SpeedSumResY = 0;

			TNode **lFNode = BNode.FarNodes->First; //Far Node
			TNode **&LastFNode = BNode.FarNodes->Last;
			for ( ; lFNode<LastFNode; lFNode++ )
			{
				TNode &FNode = **lFNode;

				//cmass circle
				BioSavar(FNode.CMp, BVortX, BVortY, SpeedSumResX, SpeedSumResY);
				SpeedSumX += SpeedSumResX; SpeedSumY += SpeedSumResY;
				BioSavar(FNode.CMm, BVortX, BVortY, SpeedSumResX, SpeedSumResY);
				SpeedSumX += SpeedSumResX; SpeedSumY += SpeedSumResY;
				BioSavarIm(FNode.CMp, BVortX, BVortY, SpeedSumResX, SpeedSumResY);
				SpeedSumX += SpeedSumResX; SpeedSumY += SpeedSumResY;
				BioSavarIm(FNode.CMm, BVortX, BVortY, SpeedSumResX, SpeedSumResY);
				SpeedSumX += SpeedSumResX; SpeedSumY += SpeedSumResY;
			}
			
			BVort.g += BVortX*(SpeedSumY+TwoInfY) - BVortY*(SpeedSumX+TwoInfX);
			BVort.g *= dfi;
			BVort.g -= CirculationAdditionDueToRotation;
		}
	}

	return 0;
}


