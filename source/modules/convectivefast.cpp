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

/****************************** MAIN FUNCTIONS ********************************/

convectivefast::convectivefast(Space *sS, double sRd2)
{
	S = sS;
	Rd2 = sRd2;
	Rd = sqrt(Rd2);

	MatrixSize = 0; 
	const_for(sS->BodyList, llbody)
	{
		MatrixSize+=(**llbody).size();
	}

	BodyMatrix = new double[sqr(MatrixSize)]; //(double*)malloc(N*N*sizeof(double));
	InverseMatrix = new double[sqr(MatrixSize)]; //(double*)malloc(N*N*sizeof(double));
	RightCol = new double[MatrixSize]; //(double*)malloc(N*sizeof(double));
	Solution = new double[MatrixSize]; //(double*)malloc(N*sizeof(double));
	ipvt = new int[MatrixSize+1]; //(int*)malloc((N+1)*sizeof(int));
	BodyMatrixOK = InverseMatrixOK = false;
}

double *convectivefast::MatrixLink()
{
	if (!BodyMatrixOK) {dbg(FillMatrix());} 
	return BodyMatrix;
}
double *convectivefast::InvMatrixLink()
{
	if (!InverseMatrixOK) {dbg(FillInverseMatrix());} 
	return InverseMatrix;
}

TVec convectivefast::SpeedSumFast(TVec p)
{
	TVec res(0, 0);
	TNode* Node = S->Tree->findNode(p);
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

inline
TVec convectivefast::BioSavar(const TObj &obj, const TVec &p)
{
	TVec dr = p - obj;
	return rotl(dr)*(obj.g / (dr.abs2() + Rd2) );
}

TVec convectivefast::SpeedSum(const TNode &Node, const TVec &p)
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
}

void convectivefast::CalcConvectiveFast()
{
	auto BottomNodes = S->Tree->getBottomNodes();

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
				obj.v += TVec(Teilor1, Teilor2) + S->InfSpeed() + SpeedSum(bnode, obj) +
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
				obj.v += TVec(Teilor1, Teilor2) + S->InfSpeed() + SpeedSum(bnode, obj) +
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
				obj.v += TVec(Teilor1, Teilor2) + S->InfSpeed() + SpeedSum(bnode, obj) +
				         TVec(TVec(Teilor3,  Teilor4)*dr_local,
				              TVec(Teilor4, -Teilor3)*dr_local);
				#undef obj
			}
		}
		#undef bnode
	}
}

void convectivefast::CalcBoundaryConvective()
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

TVec convectivefast::BoundaryConvective(const TBody &b, const TVec &p)
{
	TVec dr, res(0, 0);
	auto alist = b.List;
	double rotspeed = b.getRotation(S->Time);
//	if (!rotspeed) return res;

	const_for(alist, latt)
	{
		dr = p - *latt;
		res += (dr*latt->qatt + rotl(dr)*latt->gatt) * (rotspeed/( dr.abs2() + Rd2 ));
		if (latt->bc == bc::slip)
		{
			res += BioSavar(*latt, p);
		}
	}

	return res;
}

void convectivefast::CalcCirculationFast(bool use_inverse)
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
		const_for(body.List, latt)
		{
			latt->g = Solution[latt->eq_no];
		}
		#undef body
	}
}

void convectivefast::FillInverseMatrix()
{
	memcpy(InverseMatrix, BodyMatrix, sqr(MatrixSize)*sizeof(double));
	dbg(inverse(InverseMatrix, MatrixSize, BodyMatrix));
	InverseMatrixOK = true;
	BodyMatrixOK = false;
}

double convectivefast::ConvectiveInfluence(TVec p, const TAtt &seg, double rd)
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
}

double convectivefast::NodeInfluence(const TNode &Node, const TAtt &seg, double rd)
{
	double res = 0;

	const_for(Node.NearNodes, llnnode)
	{
		auto vlist = (**llnnode).VortexLList;
		if ( !vlist ) { continue; }

		const_for (vlist, llobj)
		{
			//if (!*llobj) {continue;}
			res+= ConvectiveInfluence((**llobj), seg, rd) * (**llobj).g;
		}
	}

	const_for(Node.FarNodes, llfnode)
	{
		#define fnode (**llfnode)
		res+= ConvectiveInfluence(fnode.CMp, seg, rd) * fnode.CMp.g;
		res+= ConvectiveInfluence(fnode.CMm, seg, rd) * fnode.CMm.g;
		#undef fnode
	}

	return res*C_1_2PI;
}

double convectivefast::AttachInfluence(const TAtt &seg, double rd)
{
	double res = 0;

	const_for(S->BodyList, llbody)
	{
		#define body (**llbody)
		if (!body.List->size_safe()) continue;
		double RotSpeed_tmp = body.getRotation(S->Time);
		if (!RotSpeed_tmp) continue;

		double res_tmp=0;
		const_for(body.List, latt)
		{
			if (latt == &seg) { res_tmp+= seg.qatt*0.5; continue; }
			res_tmp+= ConvectiveInfluence(*latt, seg, rd) * latt->g;
			TAtt seg_tmp = seg;
			seg_tmp = rotl(TVec(seg));
			seg_tmp.dl = rotl(seg.dl);
			res_tmp+= ConvectiveInfluence(*latt, seg_tmp, rd) * latt->qatt;
		}
		res+= res_tmp * RotSpeed_tmp;
		#undef body
	}

	return res * C_1_2PI;
}

void convectivefast::FillMatrix()
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

		const_for(ibody.List, latt)
		{
			int i = latt->eq_no;
			const_for(S->BodyList, lljbody)
			{
				#define jbody (**lljbody)
				const_for(jbody.List, lobj)
				{
					int j=lobj->eq_no;
					switch (latt->bc)
					{
					case bc::slip:
					case bc::noslip:
						BodyMatrix[MatrixSize*i+j] = ConvectiveInfluence(*lobj, *latt, lobj->g)*C_1_2PI;
						// lobj->g TEMPORARILY stores rd info. Assignment made 30 lines earlier.
						break;
					case bc::kutta:
						BodyMatrix[MatrixSize*i+j] = (lobj == latt) ? 1:0;
					case bc::steady:
						BodyMatrix[MatrixSize*i+j] = (llibody==lljbody)?1:0;
						break;
					case bc::inf_steady:
						BodyMatrix[MatrixSize*i+j] = 1;
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

void convectivefast::FillRightCol()
{
	double rot_sum = 0;
	const_for (S->BodyList, llbody)
	{
		#define body (**llbody)
		double tmp = 0;
		const_for(body.List, latt)
		{
			tmp+= latt->gatt;
		}
		tmp*= body.getRotation(S->Time);
		rot_sum+= tmp;
		#undef body
	}

	const_for (S->BodyList, llbody)
	{
		#define body (**llbody)
		double tmp = 0;
		const_for(body.List, latt)
		{
			tmp+= latt->g;
		}
		tmp*= body.getRotation(S->Time) - body.getRotation(S->Time-S->dt);

		#pragma omp parallel for
		const_for (body.List, latt)
		{
			TNode* Node = S->Tree->findNode(*latt);
			if (!Node) { continue; }

			TVec SegDl = latt->dl;

			switch (latt->bc)
			{
			case bc::slip:
			case bc::noslip:
				RightCol[latt->eq_no] = rotl(S->InfSpeed())*SegDl;
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

void convectivefast::SolveMatrix()
{
	if (!BodyMatrixOK)
	{
		cerr << "Matrix isn't filled!\n";
		return;
	}

	int info, one=1, N_int=MatrixSize;
	transpose(BodyMatrix, MatrixSize);
	dgesv_(&N_int,&one,BodyMatrix,&N_int,ipvt,RightCol,&N_int,&info);

	if (info)
	{
		cerr << "SolveMatrix() failed with info=" << info << endl;
		return;
	}

	for (size_t i=0; i<MatrixSize; i++)
	{
		Solution[i] = RightCol[i];
	}
}

void convectivefast::SpoilBodyMatrix() {BodyMatrixOK=false;}
void convectivefast::SpoilInverseMatrix() {InverseMatrixOK=false;}

void convectivefast::inverse(double* A, int N, double* workspace)
{
	int LWORK = N*N;
	int info;

	dgetrf_(&N,&N,A,&N,ipvt,&info);
	if (info) { cerr << "dgetrf_() failed with info=" << info << endl; return; }
	dgetri_(&N,A,&N,ipvt,workspace,&LWORK,&info);
	if (info) { cerr << "dgetri_() failed with info=" << info << endl; return; }
}

void convectivefast::transpose(double* A, int N)
{
	#pragma omp parallel for
	for (int i=0; i<MatrixSize; i++)
	{
		for (int j=i+1; j<MatrixSize; j++)
		{
			double tmp = A[i*N+j];
			A[i*N+j] = A[j*N+i];
			A[j*N+i] = tmp;
		}
	}
}

void convectivefast::SolveMatrix_inv()
{
	if (!InverseMatrixOK)
	{
		cerr << "Inverse!\n";
	}

	#pragma omp parallel for
	for (size_t i=0; i<MatrixSize; i++)
	{
		double *RowI = InverseMatrix + MatrixSize*i;
		#define SolI Solution[i]
		SolI = 0;
		for (size_t j=0; j<MatrixSize; j++)
		{
			SolI+= RowI[j]*RightCol[j];
		}
		#undef SolI
	}
}

/****** LOAD/SAVE MATRIX *******/

bool convectivefast::LoadBodyMatrix(const char* filename)
{ BodyMatrixOK = (LoadMatrix(BodyMatrix, filename) == sqr(MatrixSize)); return BodyMatrixOK; }
bool convectivefast::LoadInverseMatrix(const char* filename)
{ InverseMatrixOK = (LoadMatrix(InverseMatrix, filename) == sqr(MatrixSize)); return InverseMatrixOK; }
void convectivefast::SaveBodyMatrix(const char* filename)
{ SaveMatrix(BodyMatrix, filename); }
void convectivefast::SaveInverseMatrix(const char* filename)
{ SaveMatrix(InverseMatrix, filename); }

size_t convectivefast::LoadMatrix(double *matrix, const char* filename)
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
}

void convectivefast::SaveMatrix(double *matrix, const char* filename)
{
	FILE *fout;

	fout = fopen(filename, "w");
	if (!fout) { cerr << "Error opening file \'" << filename << "\'\n"; return; } 
	double *src = matrix;
	for (size_t i=0; i<sqr(MatrixSize); i++)
	{
		fprintf(fout, "%f ", src[i]);
		if (!((i+1)%MatrixSize)) fprintf(fout, "\n");
	}
	fclose(fout);
}

/****** BINARY LOAD/SAVE *******/

bool convectivefast::LoadBodyMatrix_bin(const char* filename)
{ BodyMatrixOK = (LoadMatrix_bin(BodyMatrix, filename) == sqr(MatrixSize)); return BodyMatrixOK; }
bool convectivefast::LoadInverseMatrix_bin(const char* filename)
{ InverseMatrixOK = (LoadMatrix_bin(InverseMatrix, filename) == sqr(MatrixSize)); return InverseMatrixOK; }
void convectivefast::SaveBodyMatrix_bin(const char* filename)
{ SaveMatrix_bin(BodyMatrix, filename); }
void convectivefast::SaveInverseMatrix_bin(const char* filename)
{ SaveMatrix_bin(InverseMatrix, filename); }

size_t convectivefast::LoadMatrix_bin(double *matrix, const char* filename)
{
	FILE *fin = fopen(filename, "rb");
	if (!fin) { return 0; }
	size_t result = fread(matrix, sizeof(double), sqr(MatrixSize), fin);
	fclose(fin);

	return result;
}

void convectivefast::SaveMatrix_bin(double *matrix, const char* filename)
{
	FILE *fout;

	fout = fopen(filename, "wb");
	fwrite(matrix, sizeof(double), sqr(MatrixSize), fout);
	fclose(fout);
}

