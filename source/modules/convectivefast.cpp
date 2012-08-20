#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <complex>

#include "mkl.h"
using namespace std;

#include "convectivefast.h"
//#define dbg(func) cout << "Doing " << #func << "... " << flush; func; cout << "done\n";
#define dbg(func) func;

/********************************** HEADER ************************************/

namespace {

Space *S;
double Rd2; //radius of discrete ^2
double Rd;

inline
TVec BioSavar(const TObj &obj, const TVec &p);
TVec SpeedSum(const TNode &Node, const TVec &p);

TVec BoundaryConvective(TBody &b, const TVec &p);

size_t N; // BodySize;
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

double ConvectiveInfluence_vortex(TVec p, const TAtt &seg, double rd);
double ConvectiveInfluence_source(TVec p, const TAtt &seg, double rd);
TVec SegmentInfluence(TVec p, const TAtt &seg, double g, double q, double rd);
TVec SegmentInfluence_linear(TVec p, const TAtt &seg, double g1, double g2);
double NodeInfluence(const TNode &Node, const TAtt &seg, double rd);
double AttachInfluence(const TAtt &seg, double rd);

size_t LoadMatrix(double *matrix, const char* filename);
void SaveMatrix(double *matrix, const char* filename);
size_t LoadMatrix_bin(double *matrix, const char* filename);
void SaveMatrix_bin(double *matrix, const char* filename);

}

/****************************** MAIN FUNCTIONS ********************************/

void InitConvectiveFast(Space *sS, double sRd2)
{
	S = sS;
	Rd2 = sRd2;
	Rd = sqrt(Rd2);

	N = 0; const_for(sS->BodyList, llbody){ N+=(**llbody).List->size(); }
	BodyMatrix = new double[N*N]; //(double*)malloc(N*N*sizeof(double));
	InverseMatrix = new double[N*N]; //(double*)malloc(N*N*sizeof(double));
	RightCol = new double[N]; //(double*)malloc(N*sizeof(double));
	Solution = new double[N]; //(double*)malloc(N*sizeof(double));
	ipvt = new int[N+1]; //(int*)malloc((N+1)*sizeof(int));
	BodyMatrixOK = InverseMatrixOK = false;
}

double *MatrixLink()
{
	if (!BodyMatrixOK) {dbg(FillMatrix());} 
	return BodyMatrix;
}
double *InvMatrixLink()
{
	if (!InverseMatrixOK) {dbg(FillInverseMatrix());} 
	return InverseMatrix;
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
	
	const_for(S->BodyList, llbody)
	{
		res += BoundaryConvective(**llbody, p);
	}
	res *= C_1_2PI;
	res += SpeedSum(*Node, p);
	res += S->InfSpeed();

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

	auto BottomNodes = GetTreeBottomNodes();
	if (!BottomNodes) {cerr << "CalcConvectiveFast() is called before tree is built"
	                        << endl; return; }

	TVec InfSpeed_tmp = S->InfSpeed();
	#pragma omp parallel for
	const_for (BottomNodes, llbnode)
	{
		#define bnode (**llbnode)

		double Teilor1, Teilor2, Teilor3, Teilor4;
		Teilor1 = Teilor2 = Teilor3 = Teilor4 = 0;

		const_for (bnode.FarNodes, llfnode)
		{
			#define fnode (**llfnode)

			TVec DistP = TVec(bnode.x, bnode.y) - fnode.CMp;
			TVec DistM = TVec(bnode.x, bnode.y) - fnode.CMm;

//			double _1_DistPabs = 1./DistP.abs2();
//			double _1_DistMabs = 1./DistM.abs2();
			double FuncP1 = fnode.CMp.g / DistP.abs2(); //Extremely complicated useless variables
			double FuncM1 = fnode.CMm.g / DistM.abs2();
			double FuncP2 = fnode.CMp.g / sqr(DistP.abs2());
			double FuncM2 = fnode.CMm.g / sqr(DistM.abs2());
			#undef fnode

			Teilor1 -= (FuncP1*DistP.ry + FuncM1*DistM.ry);
			Teilor2 += (FuncP1*DistP.rx + FuncM1*DistM.rx);
			Teilor3 += (FuncP2*DistP.ry*DistP.rx + FuncM2*DistM.ry*DistM.rx);
			Teilor4 += (FuncP2 * (sqr(DistP.ry) - sqr(DistP.rx)) + FuncM2 * (sqr(DistM.ry) - sqr(DistM.rx)));
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
				#define obj (**llobj)
				dr_local = obj - TVec(bnode.x, bnode.y);
				obj.v += TVec(Teilor1, Teilor2) + InfSpeed_tmp + SpeedSum(bnode, obj) +
				         TVec(TVec(Teilor3,  Teilor4)*dr_local,
				              TVec(Teilor4, -Teilor3)*dr_local);
				#undef obj
			}
		}

		if (bnode.HeatLList)
		{
			const_for (bnode.HeatLList, llobj)
			{
				if (!*llobj) {continue;}
				#define obj (**llobj)
				dr_local = obj - TVec(bnode.x, bnode.y);
				obj.v += TVec(Teilor1, Teilor2) + InfSpeed_tmp + SpeedSum(bnode, obj) +
				         TVec(TVec(Teilor3,  Teilor4)*dr_local,
				              TVec(Teilor4, -Teilor3)*dr_local);
				#undef obj
			}
		}

		if (bnode.StreakLList)
		{
			const_for (bnode.StreakLList, llobj)
			{
				#define obj (**llobj)
				dr_local = obj - TVec(bnode.x, bnode.y);
				obj.v += TVec(Teilor1, Teilor2) + InfSpeed_tmp + SpeedSum(bnode, obj) +
				         TVec(TVec(Teilor3,  Teilor4)*dr_local,
				              TVec(Teilor4, -Teilor3)*dr_local);
				#undef obj
			}
		}
		#undef bnode
	}
}

void CalcBoundaryConvective()
{
	if (S->VortexList)
	const_for(S->VortexList, lobj)
	{
		const_for(S->BodyList, llbody)
		{
			lobj->v += BoundaryConvective(**llbody, *lobj)*C_1_2PI;
		}
	}

	if (S->HeatList)
	const_for(S->HeatList, lobj)
	{
		const_for(S->BodyList, llbody)
		{
			lobj->v += BoundaryConvective(**llbody, *lobj)*C_1_2PI;
		}
	}

	if (S->StreakList)
	const_for(S->StreakList, lobj)
	{
		const_for(S->BodyList, llbody)
		{
			lobj->v += BoundaryConvective(**llbody, *lobj)*C_1_2PI;
		}
	}
}

namespace {
TVec BoundaryConvective(TBody &b, const TVec &p)
{
	TVec dr, res(0, 0);
	auto alist = b.AttachList;
	double rotspeed = b.RotSpeed();
//	if (!rotspeed) return res;

	const_for(alist, latt)
	{
		if ((p-*latt).abs2() < latt->dl.abs2()) return rotl(p-b.RotAxis)*b.RotSpeed()*C_2PI;
		dr = p - *latt;
		res += (dr*latt->q + rotl(dr)*latt->g) * (rotspeed/( dr.abs2() + Rd2 ));

/*		if ((p-*latt).abs2() < 4*latt->dl.abs2())
		{
			double g1 = (latt->g+b.prev(latt)->g)*0.5;
			double g2 = (latt->g+b.next(latt)->g)*0.5;
			double q1 = (latt->q+b.prev(latt)->q)*0.5;
			double q2 = (latt->q+b.next(latt)->q)*0.5;
			res+= rotl(SegmentInfluence_linear(p, *latt, g1, g2)) + SegmentInfluence_linear(p, *latt, q1, q2);
		} else
		{
			dr = p - *latt;
			res += (dr*latt->q + rotl(dr)*latt->g) * (rotspeed/( dr.abs2() + Rd2 ));
		}
*/
		//res+= SegmentInfluence(p, *latt, latt->g, latt->q, 1E-6);
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
		if (!InverseMatrixOK) { dbg(FillInverseMatrix()); }
		dbg(SolveMatrix_inv());
	}
	else
	{
		dbg(SolveMatrix());
		BodyMatrixOK = false;
	}

	const_for (S->BodyList, llbody)
	{
		#define body (**llbody)
		const_for(body.List, lobj)
		{
			lobj->g = Solution[body.att(lobj)->eq_no];
		}
		#undef body
	}
}

void FillInverseMatrix()
{
	memcpy(InverseMatrix, BodyMatrix, N*N*sizeof(double));
	dbg(inverse(InverseMatrix, N, BodyMatrix));
	InverseMatrixOK = true;
	BodyMatrixOK = false;
}

namespace {
double ConvectiveInfluence_vortex(TVec p, const TAtt &seg, double rd)
{
	complex<double> z(p.rx, -p.ry);
	complex<double> zc(seg.rx,-seg.ry);
	complex<double> dz(seg.dl.rx,-seg.dl.ry);
	complex<double> z1 = zc-dz*0.5;
	complex<double> z2 = zc+dz*0.5;

	double c1=abs(z-z1);
	double c2=abs(z-z2);
	if ((c1>=rd)&&(c2>=rd))
	{
		return -log((z-z1)/(z-z2)).real();
	} else
	if ((c1<=rd)&&(c2<=rd))
	{
		return ((seg-p)*seg.dl)/(rd*rd);
	}
	else
	{
		double a0 = seg.dl.abs2();
		double b0 = (p-seg)*seg.dl;
		double d  = sqrt(b0*b0-a0*((p-seg).abs2()-rd*rd));
		double k  = (b0+d)/a0; if ((k<=-0.5)||(k>=0.5)) k = (b0-d)/a0;
		complex<double> z3 = zc + k*dz;

		if (c1 < rd)
			return (((z1+z3)*0.5-z)*conj(z3-z1)).real()/(rd*rd) -
			       log((z-z3)/(z-z2)).real();
		else
			return -log((z-z1)/(z-z3)).real() +
			       (((z3+z2)*0.5-z)*conj(z2-z3)).real()/(rd*rd);
	}
}}

namespace {
double ConvectiveInfluence_source(TVec p, const TAtt &seg, double rd)
{
	complex<double> z(p.rx, -p.ry);
	complex<double> zc(seg.rx,-seg.ry);
	complex<double> dz(seg.dl.rx,-seg.dl.ry);
	complex<double> z1 = zc-dz*0.5;
	complex<double> z2 = zc+dz*0.5;

	double c1=abs(z-z1);
	double c2=abs(z-z2);
	if ((c1>=rd)&&(c2>=rd))
	{
		return -log((z-z1)/(z-z2)).imag();
	} else
	if ((c1<=rd)&&(c2<=rd))
	{
		return ((seg-p)*rotl(seg.dl))/(rd*rd);
	}
	else
	{
		double a0 = seg.dl.abs2();
		double b0 = (p-seg)*seg.dl;
		double d  = sqrt(b0*b0-a0*((p-seg).abs2()-rd*rd));
		double k  = (b0+d)/a0; if ((k<=-0.5)||(k>=0.5)) k = (b0-d)/a0;
		complex<double> z3 = zc + k*dz;

		if (c1 < rd)
			return -(((z1+z3)*0.5-z)*conj(z3-z1)).imag()/(rd*rd) -
			       log((z-z3)/(z-z2)).imag();
		else
			return -log((z-z1)/(z-z3)).imag() -
			       (((z3+z2)*0.5-z)*conj(z2-z3)).imag()/(rd*rd);
	}
}}

namespace {
TVec SegmentInfluence(TVec p, const TAtt &seg, double g, double q, double rd)
{
	complex<double> z(p.rx, -p.ry);
	complex<double> zc(seg.rx,-seg.ry);
	complex<double> dz(seg.dl.rx,-seg.dl.ry);
	complex<double> z1 = zc-dz*0.5;
	complex<double> z2 = zc+dz*0.5;
	complex<double> zqg(-g, q);
	complex<double> zV;
	complex<double> i(0,1);

	double c1=abs(z-z1);
	double c2=abs(z-z2);
	if ((c1>=rd)&&(c2>=rd))
	{
		zV = -i * log((z-z1)/(z-z2)) / dz;
	} else
	if ((c1<=rd)&&(c2<=rd))
	{
		zV = (zc - z) / (rd*rd);
	}
	else
	{
		double a0 = seg.dl.abs2();
		double b0 = (p-seg)*seg.dl;
		double d  = sqrt(b0*b0-a0*((p-seg).abs2()-rd*rd));
		double k  = (b0+d)/a0; if ((k<=-0.5)||(k>=0.5)) k = (b0-d)/a0;
		complex<double> z3 = zc + k*dz;

		if (c1 < rd)
			zV = ((z1+z3)*0.5 - z) / (rd*rd) -
			       i * log((z-z3)/(z-z2)) / dz;
		else
			zV = - i * log((z-z1)/(z-z3)) / dz +
			     ((z3+z2)*0.5 - z) / (rd*rd);
	}

	zV*= zqg;

	return TVec(zV.real(), zV.imag());
}}

namespace {
TVec SegmentInfluence_linear(TVec p, const TAtt &seg, double g1, double g2)
{
//	cerr << "orly?" << endl;
	complex<double> z(p.rx, p.ry);
	complex<double> zc(seg.rx, seg.ry);
	complex<double> dz(seg.dl.rx, seg.dl.ry);
	complex<double> zs1 = zc-dz*0.5;
	complex<double> zs2 = zc+dz*0.5;
	complex<double> z1 = z - zs1;
	complex<double> z2 = z - zs2;
	complex<double> zV;
	//complex<double> i(0,1);

	//cerr << z2 << "\t" << z1 << endl;
	zV = 0.5* abs(dz)/conj(dz) * ( (g2-g1) - conj(g2*z1 - g1*z2)/conj(dz)*log(conj(z2)/conj(z1)));
	//cerr << zV << endl;
	return TVec(zV.real(), zV.imag());
}}

namespace {
double NodeInfluence(const TNode &Node, const TAtt &seg, double rd)
{
	double res = 0;

	const_for(Node.NearNodes, llnnode)
	{
		auto vlist = (**llnnode).VortexLList;
		if ( !vlist ) { continue; }

		const_for (vlist, llobj)
		{
			//if (!*llobj) {continue;}
			res+= ConvectiveInfluence_vortex((**llobj), seg, rd) * (**llobj).g;
		}
	}

	const_for(Node.FarNodes, llfnode)
	{
		#define fnode (**llfnode)
		res+= ConvectiveInfluence_vortex(fnode.CMp, seg, rd) * fnode.CMp.g;
		res+= ConvectiveInfluence_vortex(fnode.CMm, seg, rd) * fnode.CMm.g;
		#undef fnode
	}

	return res*C_1_2PI;
}}

namespace {
double AttachInfluence(const TAtt &seg, double rd)
{
	double res = 0;

	const_for(S->BodyList, llbody)
	{
		#define body (**llbody)
		if (!body.AttachList->size_safe()) continue;
		double RotSpeed_tmp = body.RotSpeed();
		if (!RotSpeed_tmp) continue;

		double res_tmp=0;
		const_for(body.AttachList, latt)
		{
			if (latt == &seg) { res_tmp+= -seg.q*0.5*C_2PI; continue; }
			res_tmp+= ConvectiveInfluence_vortex(*latt, seg, rd) * latt->g;
			res_tmp+= ConvectiveInfluence_source(*latt, seg, rd) * latt->q;
		}
		res+= res_tmp * RotSpeed_tmp;
		#undef body
	}

	return res * C_1_2PI;
}}

void FillMatrix()
{
	//BodyMatrix[N*i+j]
	//RightCol[i]

	const_for(S->BodyList, llbody)
	{
		#define body (**llbody)
		const_for(body.List, lbvort)
		{
			//temporarily vort->g stores eps info.
			lbvort->g = (*body.next(lbvort)-*lbvort).abs()+
			            (*body.prev(lbvort)-*lbvort).abs();
			lbvort->g *= 0.25;
		}
		#undef body
	}

	const_for(S->BodyList, llibody)
	{
		#define ibody (**llibody)
		#pragma omp parallel for

		const_for(ibody.AttachList, latt)
		{
			int i = latt->eq_no;
			const_for(S->BodyList, lljbody)
			{
				#define jbody (**lljbody)
				const_for(jbody.List, lobj)
				{
					int j=jbody.att(lobj)->eq_no;
					switch (latt->bc)
					{
					case bc::slip:
					case bc::noslip:
						BodyMatrix[N*i+j] = ConvectiveInfluence_vortex(*lobj, *latt, lobj->g)*C_1_2PI;
						// lobj->g TEMPORARILY stores rd info. Assignment made 30 lines earlier.
						break;
					case bc::kutta:
						BodyMatrix[N*i+j] = (lobj == ibody.obj(latt)) ||
						                    (lobj == ibody.next(ibody.obj(latt))) ?
						                    1:0;
					case bc::steady:
						BodyMatrix[N*i+j] = (llibody==lljbody)?1:0;
						break;
					case bc::inf_steady:
						BodyMatrix[N*i+j] = 1;
						break;
					}
				}
				#undef jbody
			}
		}
		#undef ibody
	}

	BodyMatrixOK = true;
}

void FillRightCol()
{
	double rot_sum = 0;
	const_for (S->BodyList, llbody)
	{
		#define body (**llbody)
		double tmp = 0;
		const_for(body.AttachList, latt)
		{
			tmp+= latt->g;
		}
		tmp*= body.RotSpeed();
		rot_sum+= tmp;
		#undef body
	}

	TVec InfSpeed_tmp = S->InfSpeed();
	const_for (S->BodyList, llbody)
	{
		#define body (**llbody)
		double tmp = 0;
		const_for(body.AttachList, latt)
		{
			tmp+= latt->g;
		}
		tmp*= body.RotSpeed() - body.RotSpeed(S->Time-S->dt);

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
				RightCol[latt->eq_no] = rotl(InfSpeed_tmp)*SegDl;
				RightCol[latt->eq_no] -= NodeInfluence(*Node, *latt, Rd);
				RightCol[latt->eq_no] -= AttachInfluence(*latt, Rd);
				break;
			case bc::kutta:
			case bc::steady:
				RightCol[latt->eq_no] = -tmp +  body.g_dead;
				body.g_dead = 0;
			break;
			case bc::inf_steady:
				RightCol[latt->eq_no] = -S->gsum() - rot_sum;
				break;
			}
		}
		#undef body
	}
}

void SolveMatrix()
{
	if (!BodyMatrixOK)
	{
		cerr << "Matrix isn't filled!\n";
		return;
	}

	int info, one=1, N_int=N;
	transpose(BodyMatrix, N);
	dgesv_(&N_int,&one,BodyMatrix,&N_int,ipvt,RightCol,&N_int,&info);

	if (info)
	{
		cerr << "SolveMatrix() failed with info=" << info << endl;
		return;
	}

	for (size_t i=0; i<N; i++)
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
	for (size_t i=0; i<N; i++)
	{
		double *RowI = InverseMatrix + N*i;
		#define SolI Solution[i]
		SolI = 0;
		for (size_t j=0; j<N; j++)
		{
			SolI+= RowI[j]*RightCol[j];
		}
		#undef SolI
	}
}}

/****** LOAD/SAVE MATRIX *******/

bool LoadBodyMatrix(const char* filename)
{ BodyMatrixOK = (LoadMatrix(BodyMatrix, filename) == N*N); return BodyMatrixOK; }
bool LoadInverseMatrix(const char* filename)
{ InverseMatrixOK = (LoadMatrix(InverseMatrix, filename) == N*N); return InverseMatrixOK; }
void SaveBodyMatrix(const char* filename)
{ SaveMatrix(BodyMatrix, filename); }
void SaveInverseMatrix(const char* filename)
{ SaveMatrix(InverseMatrix, filename); }

namespace {
size_t LoadMatrix(double *matrix, const char* filename)
{
	FILE *fin;

	fin = fopen(filename, "r");
	if (!fin) { cerr << "No file called \'" << filename << "\'\n"; return 0; } 
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
	for (size_t i=0; i<N*N; i++)
	{
		fprintf(fout, "%f ", src[i]);
		if (!((i+1)%N)) fprintf(fout, "\n");
	}
	fclose(fout);
}}

/****** BINARY LOAD/SAVE *******/

bool LoadBodyMatrix_bin(const char* filename)
{ BodyMatrixOK = (LoadMatrix_bin(BodyMatrix, filename) == N*N); return BodyMatrixOK; }
bool LoadInverseMatrix_bin(const char* filename)
{ InverseMatrixOK = (LoadMatrix_bin(InverseMatrix, filename) == N*N); return InverseMatrixOK; }
void SaveBodyMatrix_bin(const char* filename)
{ SaveMatrix_bin(BodyMatrix, filename); }
void SaveInverseMatrix_bin(const char* filename)
{ SaveMatrix_bin(InverseMatrix, filename); }

namespace {
size_t LoadMatrix_bin(double *matrix, const char* filename)
{
	FILE *fin = fopen(filename, "rb");
	if (!fin) { return 0; }
	size_t result = fread(matrix, sizeof(double), N*N, fin);
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
