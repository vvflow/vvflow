#include "convective.h"
#include "iostream"

using namespace std;

//#define M_1_PI 0.3183098861 	// = 1/PI
#define M_1_2PI 0.159154943 	// = 1/(2*PI)
#define M_2PI 6.283185308 		// = 2*PI

/********************* HEADER ****************************/

namespace {

Space *ConvectiveFast_S;
double ConvectiveFast_Eps;
double ConvectiveFast_InfSpeedX;
double ConvectiveFast_InfSpeedY;
double ConvectiveFast_RotationG;

void BioSavar(TVortex *vort, double px, double py, double &resx, double &resy);
void BioSavarIm(TVortex *vort, double px, double py, double &resx, double &resy);
void SpeedSum(TNode *Node, double px, double py, double &resx, double &resy);

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
	SpeedSum(List, px, py, 1, 0, resx, resy);
	return 0;
}*/

namespace {
void BioSavar(TVortex *Vort, double px, double py, double &resx, double &resy)
{
	double drx, dry, drabs2;
	double multiplier;

	drx = px - Vort->rx;
	dry = py - Vort->ry;
	drabs2 = drx*drx + dry*dry;
	multiplier = Vort->g / ( drabs2 + ConvectiveFast_Eps ); // 1/2PI is in flowmove
	resx = -dry * multiplier;
	resy =  drx * multiplier;
}}

namespace {
void BioSavarIm(TVortex *Vort, double px, double py, double &resx, double &resy)
{
	double drx, dry, drabs2;
	double multiplier;
	double vortrabs2 = (Vort->rx*Vort->rx + Vort->ry*Vort->ry);
	if (!vortrabs2) { resx = resy = 0; return; }
	double vort1rabs2 = 1/vortrabs2;

	drx = px - Vort->rx * vort1rabs2;
	dry = py - Vort->ry * vort1rabs2;
	drabs2 = drx*drx + dry*dry;
	multiplier = Vort->g / ( drabs2 + ConvectiveFast_Eps ); // 1/2PI is in flowmove
	resx =  dry * multiplier;
	resy = -drx * multiplier;
}}

namespace {
void SpeedSum(TNode *Node, double px, double py, double &resx, double &resy)
{
	double drx, dry, drabs2;
	double multiplier;

	int i, j, nnlsize; //Near nodes list size
	TNode **lNNode = (TNode**)Node->NearNodes->Elements; //link to NearNode link
	TNode *NNode;
	nnlsize = Node->NearNodes->size;
	
	resx = resy = 0;
	for ( i=0; i<nnlsize; i++ )
	{
		NNode = *lNNode;
		if ( !NNode->VortexLList ) { lNNode++; continue; }
		
		TVortex** lVort = (TVortex**)NNode->VortexLList->Elements;
		TVortex* Vort;
		int lsize = NNode->VortexLList->size;
		for ( j=0; j<lsize; j++ )
		{
			Vort = *lVort;
			drx = px - Vort->rx;
			dry = py - Vort->ry;
			drabs2 = drx*drx + dry*dry;
			multiplier = Vort->g / ( drabs2 + ConvectiveFast_Eps ); // 1/2PI is in flowmove
			resx -= dry * multiplier;
			resy += drx * multiplier;
			lVort++;
		}
		lNNode++;
	}

	/*int fnlsize; //Far nodes list size
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
	}*/ //this block is useless, it was working instead of Teilor
}}

int CalcConvectiveFast()
{
	int i, j, lsize;
	double SpeedSumResX, SpeedSumResY;
	double Teilor1, Teilor2, Teilor3, Teilor4;

	//initialization of InfSpeed & Rotation
	if (ConvectiveFast_S->InfSpeedX) { ConvectiveFast_InfSpeedX = M_2PI*ConvectiveFast_S->InfSpeedX(ConvectiveFast_S->Time); } 
	else { ConvectiveFast_InfSpeedX = 0; }
	if (ConvectiveFast_S->InfSpeedY) { ConvectiveFast_InfSpeedY = M_2PI*ConvectiveFast_S->InfSpeedY(ConvectiveFast_S->Time); } 
	else { ConvectiveFast_InfSpeedY = 0; }
	if (ConvectiveFast_S->RotationV) { ConvectiveFast_RotationG = M_2PI*ConvectiveFast_S->RotationV(ConvectiveFast_S->Time); } 
	else { ConvectiveFast_RotationG = 0; }
	
	TlList *BottomNodes = GetTreeBottomNodes();
	if ( !BottomNodes ) return -1;
	int bnlsize = BottomNodes->size; //Bottom nodes list size
	TNode** lBNode = (TNode**)BottomNodes->Elements; //Link to Bottom Node Link
	TNode* BNode; //Bottom Node Link

	for ( i=0; i<bnlsize; i++ )
	{
		BNode = *lBNode;
		//if ( !BNode->VortexLList ) { lBNode++; continue; }
		
		double DistPx, DistPy; //Distance between current node center and positive center of mass of far node 
		double DistMx, DistMy;
		double FuncP1, FuncM1; //Extremely complicated useless variables
		double FuncP2, FuncM2;
		
		Teilor1 = Teilor2 = Teilor3 = Teilor4 = 0;
		
		TNode** lFarNode = (TNode**)BNode->FarNodes->Elements; //Link to FarNode link
		TNode* FarNode; //FarNode link
		int fnlsize = BNode->FarNodes->size; //far nodes list size
		for ( j=0; j<fnlsize; j++ )
		{
			FarNode = *lFarNode;
			DistPx = BNode->x - FarNode->CMp.rx;
			DistPy = BNode->y - FarNode->CMp.ry;
			DistMx = BNode->x - FarNode->CMm.rx;
			DistMy = BNode->y - FarNode->CMm.ry;
			
			double _1_DistPabs = 1/(DistPx*DistPx + DistPy*DistPy);
			double _1_DistMabs = 1/(DistMx*DistMx + DistMy*DistMy);
			FuncP1 = FarNode->CMp.g * _1_DistPabs; //Extremely complicated useless variables
			FuncM1 = FarNode->CMm.g * _1_DistMabs;
			FuncP2 = FuncP1 * _1_DistPabs;
			FuncM2 = FuncM1 * _1_DistMabs;
			
			Teilor1 -= (FuncP1*DistPy + FuncM1*DistMy);
			Teilor2 += (FuncP1*DistPx + FuncM1*DistMx);
			Teilor3 += (FuncP2*DistPy*DistPx + FuncM2*DistMy*DistMx);
			Teilor4 += (FuncP2 * (DistPy*DistPy - DistPx*DistPx) + FuncM2 * (DistMy*DistMy - DistMx*DistMx));
			
			lFarNode++;
		}
		Teilor3 *= 2;
		/*
		Teilor1 *= M_1_2PI;
		Teilor2 *= M_1_2PI;
		Teilor3 *= M_1_PI;
		Teilor4 *= M_1_2PI;
		*/
		
		TVortex** lobj; //link to object: either vortex or heat particle
		TVortex* obj;  // object link
		int olsize;
		double LocaldRx, LocaldRy;
		double rabs2, multiplier;
		#define SpeedSumCircle(List) 														\
			lobj = (TVortex**)(List)->Elements; 											\
			olsize = (List)->size; /*object list size*/										\
			for ( j=0; j<olsize; j++ ) 														\
			{																				\
				obj = *lobj; 																\
				if ( ConvectiveFast_RotationG ) 											\
					multiplier = ConvectiveFast_RotationG/(obj->rx*obj->rx + obj->ry*obj->ry + ConvectiveFast_Eps); \
				else multiplier = 0; 														\
				LocaldRx = obj->rx - BNode->x; 												\
				LocaldRy = obj->ry - BNode->y; 												\
				SpeedSum(BNode, obj->rx, obj->ry, SpeedSumResX, SpeedSumResY); 				\
				obj->vx += Teilor1 + Teilor3*LocaldRx + Teilor4*LocaldRy + 					\
							SpeedSumResX + ConvectiveFast_InfSpeedX - obj->ry*multiplier; 	\
				obj->vy += Teilor2 + Teilor4*LocaldRx - Teilor3*LocaldRy + 					\
							SpeedSumResY + ConvectiveFast_InfSpeedY + obj->rx*multiplier; 	\
				lobj++; 																	\
			} 																				
		
		if ( BNode->VortexLList ) { SpeedSumCircle(BNode->VortexLList); }
		if ( BNode->HeatLList ) { SpeedSumCircle(BNode->HeatLList); }
		
		lBNode++;
	}

	return 0;
}

int CalcCirculationFast()
{
	if ( !ConvectiveFast_S->BodyList ) return -1;

	int i, j, k, m, lsize;
	double SpeedSumResX, SpeedSumResY;
	double SpeedSumX, SpeedSumY;
	double dfi = M_2PI/ConvectiveFast_S->BodyList->size;
	double SqrOfHalfDfi = dfi*dfi*0.25;
	double gt = 0;


	if (ConvectiveFast_S->InfSpeedX) { ConvectiveFast_InfSpeedX = ConvectiveFast_S->InfSpeedX(ConvectiveFast_S->Time); } 
	else { ConvectiveFast_InfSpeedX = 0; }
	if (ConvectiveFast_S->InfSpeedY) { ConvectiveFast_InfSpeedY = ConvectiveFast_S->InfSpeedY(ConvectiveFast_S->Time); } 
	else { ConvectiveFast_InfSpeedY = 0; }
	if (ConvectiveFast_S->RotationV) { ConvectiveFast_RotationG = M_2PI*ConvectiveFast_S->RotationV(ConvectiveFast_S->Time); } 
	else { ConvectiveFast_RotationG = 0; }
	double CirculationAdditionDueToRotation = ConvectiveFast_RotationG/ConvectiveFast_S->BodyList->size;


	TlList *BottomNodes = GetTreeBottomNodes();
	if ( !BottomNodes ) return -1;
	int bnlsize = BottomNodes->size; //Bottom nodes list size
	TNode** lBNode = (TNode**)BottomNodes->Elements; // link to bottom node link
	TNode* BNode; //bottom node link

	for ( i=0; i<bnlsize; i++ )
	{
		BNode = *lBNode;
		//cout << "BNode " << BNode->BodyLList << endl;
		if ( !BNode->BodyLList ) { lBNode++; continue; }
		
		TVortex** lbvort = (TVortex**)BNode->BodyLList->Elements; //link to body vortex link
		TVortex* bvort; //body vortex link
		double bvortx, bvorty;
		int blsize = BNode->BodyLList->size; //body list size
		
		for ( j=0; j<blsize; j++ )
		{
			bvort = *lbvort;
			bvortx = bvort->rx;
			bvorty = bvort->ry;
			bvort->g = 0;
			SpeedSumX = SpeedSumY = 0;
			
			TNode **lNNode = (TNode**)BNode->NearNodes->Elements;
			TNode *NNode;
			int nnlsize = BNode->NearNodes->size;
			for ( k=0; k<nnlsize; k++ )
			{
				NNode = *lNNode;
				
				if ( !NNode->VortexLList ) { lNNode++; continue; }
				//vortrex atan circle
				//weak place
				TVortex **lobj = (TVortex**)NNode->VortexLList->Elements;
				TVortex *obj;
				int olsize = NNode->VortexLList->size;
				double AtanSum=0;
				for ( m=0; m<olsize; m++ )
				{
					obj = *lobj;
					double ScalarMult = bvortx*obj->rx + bvorty*obj->ry;
					double ObjRSqr = obj->rx*obj->rx + obj->ry*obj->ry;
					double NumDifferx = obj->rx - bvortx;
					double NumDiffery = obj->ry - bvorty;
					double NumDifferSqr = NumDifferx*NumDifferx + NumDiffery*NumDiffery;
					double Num1DifferSqr = 1/NumDifferSqr;
					double Obj1Rsqr = 1/ObjRSqr;
					int flagis = 1;
					
					if ( ObjRSqr < 1 )  flagis = -1;
					if ( NumDifferSqr > SqrOfHalfDfi*120)
					{
						AtanSum += -flagis*obj->g * (ObjRSqr-1)*Num1DifferSqr;
					}
					else
					{
						AtanSum += flagis * ( atan2( ((1-ScalarMult)*dfi), (1 + ObjRSqr - 2*ScalarMult - SqrOfHalfDfi) )
								-atan2( ((1-ScalarMult*Obj1Rsqr)*dfi), (1 + (1 - 2*ScalarMult)*Obj1Rsqr - SqrOfHalfDfi) )
								) * obj->g / dfi; 
					}
					
					
					/*BioSavar(obj, bvortx, bvorty, SpeedSumResX, SpeedSumResY);
					SpeedSumX += SpeedSumResX; SpeedSumY += SpeedSumResY;
					BioSavarIm(obj, bvortx, bvorty, SpeedSumResX, SpeedSumResY);
					SpeedSumX += SpeedSumResX; SpeedSumY += SpeedSumResY;*/ //useless, instead of atan
					
					
					lobj++;
				}
				bvort->g += AtanSum*M_1_2PI; //*dfi at the end
				lNNode++;
			}
			
			TNode **lFNode = (TNode**)BNode->FarNodes->Elements; //link to farnode link
			TNode *FNode; //far node link
			int fnlsize = BNode->FarNodes->size;  //far nodes list size
			for ( k=0; k<fnlsize; k++ )
			{
				FNode = *lFNode;
				
				//cmass circle
				BioSavar(&FNode->CMp, bvortx, bvorty, SpeedSumResX, SpeedSumResY);
				SpeedSumX += SpeedSumResX; SpeedSumY += SpeedSumResY;
				BioSavar(&FNode->CMm, bvortx, bvorty, SpeedSumResX, SpeedSumResY);
				SpeedSumX += SpeedSumResX; SpeedSumY += SpeedSumResY;
				BioSavarIm(&FNode->CMp, bvortx, bvorty, SpeedSumResX, SpeedSumResY);
				SpeedSumX += SpeedSumResX; SpeedSumY += SpeedSumResY;
				BioSavarIm(&FNode->CMm, bvortx, bvorty, SpeedSumResX, SpeedSumResY);
				SpeedSumX += SpeedSumResX; SpeedSumY += SpeedSumResY;

				lFNode++;
			}
			
			bvort->g += M_1_2PI*(SpeedSumY*bvortx - SpeedSumX*bvorty);
			bvort->g += 2*(-bvorty*ConvectiveFast_InfSpeedX + bvortx*ConvectiveFast_InfSpeedY); //InfSpeed
			bvort->g *=dfi;
			bvort->g -= CirculationAdditionDueToRotation;
			//cout << bvort->g << endl;
			lbvort++;
		}
		
		lBNode++;
	}
}
