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

convectivefast::convectivefast(Space *sS, double sRd)
{
	S = sS;
	Rd = sRd;
	Rd2 = Rd*Rd;

	MatrixSize = 0; 
	const_for(sS->BodyList, llbody)
	{
		MatrixSize+=(**llbody).size();
	}

	matrix = new Matrix(MatrixSize);
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

//	if (!rotspeed) return res;
	const_for(alist, latt)
	{
		if ((p-*latt).abs2() < latt->dl.abs2())
		{
			TVec Vs1 = b.MotionSpeed_slae + b.RotationSpeed_slae * rotl(latt->corner - (b.Position + b.deltaPosition));
			double g1 = -Vs1 * latt->dl;
			double q1 = -rotl(Vs1) * latt->dl; 

			TVec Vs2 = b.MotionSpeed_slae + b.RotationSpeed_slae * rotl(latt->corner + latt->dl - (b.Position + b.deltaPosition));
			double g2 = -Vs2 * latt->dl;
			double q2 = -rotl(Vs2) * latt->dl; 

			res+= (rotl(SegmentInfluence_linear_source(p, *latt, g1, g2)) + SegmentInfluence_linear_source(p, *latt, q1, q2));
		} else
		{
			TVec Vs = b.MotionSpeed_slae + b.RotationSpeed_slae * rotl(*latt - (b.Position + b.deltaPosition));
			double g = -Vs * latt->dl;
			double q = -rotl(Vs) * latt->dl; 
			dr = p - *latt;
			res += (dr*q + rotl(dr)*g) /(dr.abs2() + Rd2);
		}

		//res+= SegmentInfluence(p, *latt, latt->g, latt->q, 1E-6);
		if (latt->bc == bc::slip)
		{
			res += BioSavar(*latt, p);
		}
	}

	return res;
}

void convectivefast::CalcCirculationFast(bool use_inverse)
{
	//FIXME auto determine when to use inverse

	FillRightCol();
	if ((use_inverse && !matrix->inverseMatrixIsOk() && !matrix->bodyMatrixIsOk()) || (!use_inverse && !matrix->bodyMatrixIsOk()))
		FillMatrix();
	matrix->solveUsingInverseMatrix(use_inverse);
}

double convectivefast::_2PI_Xi_g(TVec p, const TAtt &seg, double rd) // in doc 2\pi\Xi_\gamma (1.7)
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

double convectivefast::_2PI_Xi_q(TVec p, const TAtt &seg, double rd) // in doc 2\pi\Xi_q (1.8)
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
}

void convectivefast::_2PI_A123(const TAtt &seg, const TBody &b, double *A1, double *A2, double *A3)
{
	*A1 = *A2 = *A3 = 0;
	if (!b.kx && !b.ky && !b.ka && !b.getRotation(S->Time) && b.getMotion(S->Time).iszero()) return;
	const_for(b.List, latt)
	{
		double _2piXi_g = _2PI_Xi_g(*latt, seg, latt->dl.abs()*0.25);
		double _2piXi_q = _2PI_Xi_q(*latt, seg, latt->dl.abs()*0.25);
		TVec Xi(_2piXi_g, _2piXi_q);
		TVec r0 = *latt - (b.Position + b.deltaPosition);
		*A1 -= Xi*latt->dl;
		*A2 -= rotl(Xi)*latt->dl;
		*A3 -= rotl(Xi) * TVec(latt->dl * r0, rotl(latt->dl)*r0);
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
			res+= _2PI_Xi_g((**llobj), seg, rd) * (**llobj).g;
		}
	}

	const_for(Node.FarNodes, llfnode)
	{
		#define fnode (**llfnode)
		res+= _2PI_Xi_g(fnode.CMp, seg, rd) * fnode.CMp.g;
		res+= _2PI_Xi_g(fnode.CMm, seg, rd) * fnode.CMm.g;
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

		double res_tmp=0;
		const_for(body.List, latt)
		{
			TVec Vs = body.MotionSpeed_slae + body.RotationSpeed_slae * rotl(*latt - (body.Position + body.deltaPosition));
			if (latt == &seg) { res_tmp+= -(-rotl(Vs) * latt->dl)*0.5*C_2PI; continue; }
			res+= _2PI_Xi_g(*latt, seg, rd) * (-Vs * latt->dl);
			res+= _2PI_Xi_q(*latt, seg, rd) * (-rotl(Vs) * latt->dl);
		}
		#undef body
	}

	return res * C_1_2PI;
}

TVec SegmentInfluence_linear_source(TVec p, const TAtt &seg, double q1, double q2)
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
	zV = ( (q2-q1) - conj(q2*z1 - q1*z2)/conj(dz)*log(conj(z2)/conj(z1))) / conj(dz);
	//cerr << zV << endl;
	return TVec(zV.real(), zV.imag());
}

void convectivefast::FillMatrix()
{
	//BodyMatrix[N*i+j]
	//RightCol[i]

	const_for(S->BodyList, llibody)
	{
		#define ibody (**llibody)
		#pragma omp parallel for

		const_for(ibody.List, latt)
		{
			int eq = latt->eq_no;
			auto boundaryCondition = latt->bc;
			*matrix->solutionAtIndex(eq) = &latt->g;

			const_for(S->BodyList, lljbody)
			{
				#define jbody (**lljbody)
				const_for(jbody.List, lobj)
				{
					int j=lobj->eq_no;
					switch (boundaryCondition)
					{
					case bc::slip:
					case bc::noslip:
						*matrix->objectAtIndex(eq, j) = _2PI_Xi_g(*lobj, *latt, lobj->dl.abs()*0.5)*C_1_2PI;
						break;
					case bc::kutta:
						*matrix->objectAtIndex(eq, j) = (lobj == latt) ? 1:0;
					case bc::steady:
						*matrix->objectAtIndex(eq, j) = (llibody==lljbody)?1:0;
						break;
					case bc::inf_steady:
						*matrix->objectAtIndex(eq, j) = 1;
						break;
					}
				}

				double A1, A2, A3;
				_2PI_A123(*latt, jbody, &A1, &A2, &A3);
				*matrix->objectAtIndex(eq, jbody.eq_forces_no+0) = A1;
				*matrix->objectAtIndex(eq, jbody.eq_forces_no+1) = A2;
				*matrix->objectAtIndex(eq, jbody.eq_forces_no+2) = A3;
				#undef jbody
			}
		}

		//FIXME forces equations
		#undef ibody
	}

	matrix->markBodyMatrixAsFilled();
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
			tmp+= 0;//FIXME latt->gatt;
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
				*matrix->rightColAtIndex(latt->eq_no) = rotl(S->InfSpeed())*SegDl;
				*matrix->rightColAtIndex(latt->eq_no) -= NodeInfluence(*Node, *latt, Rd);
				*matrix->rightColAtIndex(latt->eq_no) -= AttachInfluence(*latt, Rd);
				break;
			case bc::kutta:
			case bc::steady:
				*matrix->rightColAtIndex(latt->eq_no) = -tmp +  body.g_dead;
				body.g_dead = 0;
			break;
			case bc::inf_steady:
				*matrix->rightColAtIndex(latt->eq_no) = -S->gsum() - rot_sum;
				break;
			}
		}
		#undef body
	}
}

