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

convectivefast::convectivefast(Space *sS)
{
	S = sS;

	MatrixSize = 0; 
	const_for(sS->BodyList, llbody)
	{
		MatrixSize+=(**llbody).size()+3;
	}

	matrix = new Matrix(MatrixSize);
}

TVec convectivefast::SpeedSumFast(TVec p)
{
	TVec res(0, 0);
	TSortedNode* Node = S->Tree->findNode(p);
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
	return rotl(dr)*(obj.g / (dr.abs2() + sqr(1./obj._1_eps)) );
}

TVec convectivefast::SpeedSum(const TSortedNode &Node, const TVec &p)
{
	TVec res(0, 0);

	const_for (Node.NearNodes, llnnode)
	{
		for (TObj *lobj = (**llnnode).vRange.first; lobj < (**llnnode).vRange.last; lobj++)
		{
			if (!lobj->g) continue;
			res+= BioSavar(*lobj, p); 
		}
	}

	res *= C_1_2PI;
	return res;
}

void convectivefast::CalcConvectiveFast()
{
	auto BottomNodes = S->Tree->getBottomNodes();

	#pragma omp parallel for schedule(dynamic, 10)
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

		TVec dr_local, nodeCenter(bnode.x, bnode.y);
		
		for (TObj *lobj = bnode.vRange.first; lobj < bnode.vRange.last; lobj++)
		{
			if (!lobj->g) {continue;}
			dr_local = *lobj - nodeCenter;
			lobj->v += TVec(Teilor1, Teilor2) + S->InfSpeed() + SpeedSum(bnode, *lobj) +
			         TVec(TVec(Teilor3,  Teilor4)*dr_local,
			              TVec(Teilor4, -Teilor3)*dr_local);
		}

		for (TObj *lobj = bnode.hRange.first; lobj < bnode.hRange.last; lobj++)
		{
			if (!lobj->g) {continue;}
			dr_local = *lobj - nodeCenter;
			lobj->v += TVec(Teilor1, Teilor2) + S->InfSpeed() + SpeedSum(bnode, *lobj) +
			         TVec(TVec(Teilor3,  Teilor4)*dr_local,
			              TVec(Teilor4, -Teilor3)*dr_local);
		}

		for (TObj *lobj = bnode.sRange.first; lobj < bnode.sRange.last; lobj++)
		{
			dr_local = *lobj - nodeCenter;
			lobj->v += TVec(Teilor1, Teilor2) + S->InfSpeed() + SpeedSum(bnode, *lobj) +
			         TVec(TVec(Teilor3,  Teilor4)*dr_local,
			              TVec(Teilor4, -Teilor3)*dr_local);
		}
		#undef bnode
	}
}

void convectivefast::CalcBoundaryConvective()
{
	const_for(S->BodyList, llbody)
	{
		bool calcBC = (!(**llbody).MotionSpeed_slae.iszero() || fabs((**llbody).RotationSpeed_slae) > 1E-10);
		bool calcBCS = false;
		const_for((**llbody).List, latt) { if (latt->bc == bc::slip) {calcBCS = true; break;}}

		if (!calcBC && !calcBCS) continue;

		if (S->VortexList)
		{
			#pragma omp parallel for
			const_for(S->VortexList, lobj)
			{
				if (calcBC) lobj->v += BoundaryConvective(**llbody, *lobj)*C_1_2PI;
				if (calcBCS) lobj->v += BoundaryConvectiveSlip(**llbody, *lobj)*C_1_2PI;
			}
		}

		if (S->HeatList)
		{
			#pragma omp parallel for
			const_for(S->HeatList, lobj)
			{
				if (calcBC) lobj->v += BoundaryConvective(**llbody, *lobj)*C_1_2PI;
				if (calcBCS) lobj->v += BoundaryConvectiveSlip(**llbody, *lobj)*C_1_2PI;
			}
		}

		if (S->StreakList)
		{
			#pragma omp parallel for
			const_for(S->StreakList, lobj)
			{
				if (calcBC) lobj->v += BoundaryConvective(**llbody, *lobj)*C_1_2PI;
				if (calcBCS) lobj->v += BoundaryConvectiveSlip(**llbody, *lobj)*C_1_2PI;
			}
		}
	}
}

TVec convectivefast::BoundaryConvective(const TBody &b, const TVec &p)
{
	TVec res(0, 0);
	auto alist = b.List;

	const_for(alist, latt)
	{
		double drabs2 = (p-*latt).abs2();
		if (drabs2 < latt->dl.abs2())
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
			TVec dr = p - *latt;
			res += (dr*q + rotl(dr)*g) / drabs2;
		}
		//res+= SegmentInfluence(p, *latt, latt->g, latt->q, 1E-6);
	}

	return res;
}


TVec convectivefast::BoundaryConvectiveSlip(const TBody &b, const TVec &p)
{
	TVec dr, res(0, 0);
	auto alist = b.List;

	const_for(alist, latt)
	{
		if (latt->bc == bc::slip)
		{
			res += BioSavar(*latt, p);
		}
	}

	return res;
}


bool convectivefast::canUseInverse()
{
	//Algorithm:
	// if Nb <= 1 -> use_inverse=TRUE
	// else check bodies
	//     if any kx, ky, ka >= 0 -> use_inverse=FALSE
	//     else if for any pair of bodies (Vo!=0) and (Vo or R*) differ -> use_inverse=FALSE
	//     else if for any pair of bodies Vx or Vy differ -> use_inverse=FALSE
	//     else use_inverse=TRUE
	if (S->BodyList->size() <= 1) return true;

	const_for(S->BodyList, llbody1)
	{
		TBody *lbody1 = *llbody1;

		if (lbody1->kx >= 0) return false;
		if (lbody1->ky >= 0) return false;
		if (lbody1->ka >= 0) return false;

		const_for(S->BodyList, llbody2)
		{
			TBody *lbody2 = *llbody2;
			if ((lbody1->getRotation()!=0) || (lbody2->getRotation()!=0))
			{
				if (lbody1->getRotation() != lbody2->getRotation()) return false;
				if ((lbody1->Position+lbody1->deltaPosition) != (lbody2->Position+lbody2->deltaPosition)) return false;
			}

			if (lbody1->getSpeedX() != lbody2->getSpeedX()) return false;
			if (lbody1->getSpeedY() != lbody2->getSpeedY()) return false;
		}
	}

	return true;
}

void convectivefast::CalcCirculationFast()
{
	bool use_inverse = canUseInverse();

	if (matrix->bodyMatrixIsOk())
		FillMatrix(true);
	else
		FillMatrix(false);

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

double convectivefast::_2PI_Xi_q(TVec &p, const TAtt &seg, double rd) // in doc 2\pi\Xi_q (1.8)
{
	if (&p == &seg) { TVec pnew = p+rotl(seg.dl)*0.001; return _2PI_Xi_q(pnew, seg, rd); }
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

void convectivefast::_2PI_A123(const TAtt &seg, const TBody &b, double *_2PI_A1, double *_2PI_A2, double *_2PI_A3)
{
	*_2PI_A1 = C_2PI * seg.dl.ry;
	*_2PI_A2 = -C_2PI * seg.dl.rx;
	*_2PI_A3 = 0;
	if ((b.ka<0) && (!b.getRotation())) 
	{
		//FIXME econome time. uncomment return
		//fprintf(stderr, "ret:\t%lf\t%lf\n", seg.corner.rx, seg.corner.ry);
		return;
	}
	const_for(b.List, latt)
	{
		double _2piXi_g = _2PI_Xi_g(*latt, seg, seg.dl.abs()*0.25);
		double _2piXi_q = _2PI_Xi_q(*latt, seg, seg.dl.abs()*0.25);
		TVec Xi(_2piXi_g, _2piXi_q);
		TVec r0 = *latt - (b.Position + b.deltaPosition);
//		*A1 -= Xi*latt->dl;
//		*A2 -= rotl(Xi)*latt->dl;
		*_2PI_A3 -= Xi * TVec(rotl(r0) * latt->dl, -latt->dl*r0);
	}
}

double convectivefast::NodeInfluence(const TSortedNode &Node, const TAtt &seg)
{
	double res = 0;

	const_for(Node.NearNodes, llnnode)
	{
		for (TObj *lobj = (**llnnode).vRange.first; lobj < (**llnnode).vRange.last; lobj++)
		{
			if (!lobj->g) {continue;}
			res+= _2PI_Xi_g(*lobj, seg, seg.dl.abs()*0.25) * lobj->g;
		}
	}

	const_for(Node.FarNodes, llfnode)
	{
		#define fnode (**llfnode)
		res+= _2PI_Xi_g(fnode.CMp, seg, 0) * fnode.CMp.g;
		res+= _2PI_Xi_g(fnode.CMm, seg, 0) * fnode.CMm.g;
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

TVec convectivefast::SegmentInfluence_linear_source(TVec p, const TAtt &seg, double q1, double q2)
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

void convectivefast::fillSlipEquationForSegment(TAtt* seg, bool rightColOnly)
{
	int seg_eq_no = seg->eq_no; //equation number for current segment
	if (!rightColOnly)
	const_for(S->BodyList, lljbody)
	{
		#define jbody (**lljbody)
		const_for(jbody.List, lobj)
		{
			*matrix->objectAtIndex(seg_eq_no, lobj->eq_no) = _2PI_Xi_g(lobj->corner, *seg, seg->dl.abs()*0.25)*C_1_2PI;
		}

		double _2PI_A1, _2PI_A2, _2PI_A3;
		_2PI_A123(*seg, jbody, &_2PI_A1, &_2PI_A2, &_2PI_A3);
		*matrix->objectAtIndex(seg_eq_no, jbody.eq_forces_no+0) = _2PI_A1*C_1_2PI;
		*matrix->objectAtIndex(seg_eq_no, jbody.eq_forces_no+1) = _2PI_A2*C_1_2PI;
		*matrix->objectAtIndex(seg_eq_no, jbody.eq_forces_no+2) = _2PI_A3*C_1_2PI;

		#undef jbody
	}

	//place solution pointer
	*matrix->solutionAtIndex(seg_eq_no) = &seg->g;

	//right column
	//influence of infinite speed
	*matrix->rightColAtIndex(seg_eq_no) = rotl(S->InfSpeed())*seg->dl;
	//influence of all free vortices
	TSortedNode* Node = S->Tree->findNode(*seg);
	*matrix->rightColAtIndex(seg_eq_no) -= NodeInfluence(*Node, *seg);
}

void convectivefast::fillZeroEquationForSegment(TAtt* seg, bool rightColOnly)
{
	int seg_eq_no = seg->eq_no; //equation number for current segment
	if (!rightColOnly)
	const_for(S->BodyList, lljbody)
	{
		#define jbody (**lljbody)
		const_for(jbody.List, lobj)
		{
			*matrix->objectAtIndex(seg_eq_no, lobj->eq_no) = 0;
		}

		*matrix->objectAtIndex(seg_eq_no, jbody.eq_forces_no+0) = 0;
		*matrix->objectAtIndex(seg_eq_no, jbody.eq_forces_no+1) = 0;
		*matrix->objectAtIndex(seg_eq_no, jbody.eq_forces_no+2) = 0;

		#undef jbody
	}
	//the only non-zero value
	*matrix->objectAtIndex(seg_eq_no, seg_eq_no) = 1;
	//place solution pointer
	*matrix->solutionAtIndex(seg_eq_no) = &seg->g;
	//right column
	*matrix->rightColAtIndex(seg_eq_no) = 0;
}

void convectivefast::fillSteadyEquationForSegment(TAtt* seg, bool rightColOnly)
{
	TBody* lbody = seg->body;
	int seg_eq_no = seg->eq_no; //equation number for current segment
	if (!rightColOnly)
	const_for(S->BodyList, lljbody)
	{
		#define jbody (**lljbody)
		const_for(jbody.List, lobj)
		{
			*matrix->objectAtIndex(seg_eq_no, lobj->eq_no) = (lbody==*lljbody)? 1 : 0;
		}

		*matrix->objectAtIndex(seg_eq_no, jbody.eq_forces_no+0) = 0;
		*matrix->objectAtIndex(seg_eq_no, jbody.eq_forces_no+1) = 0;
		*matrix->objectAtIndex(seg_eq_no, jbody.eq_forces_no+2) = (lbody==*lljbody) ? 2*lbody->getArea() : 0;

		#undef jbody
	}

	//place solution pointer
	*matrix->solutionAtIndex(seg_eq_no) = &seg->g;

	//right column
	*matrix->rightColAtIndex(seg_eq_no) = lbody->g_dead + 2*lbody->getArea()*lbody->RotationSpeed_slae_prev;
	lbody->g_dead = 0;
}

void convectivefast::fillInfSteadyEquationForSegment(TAtt* seg, bool rightColOnly)
{
	int seg_eq_no = seg->eq_no; //equation number for current segment
	if (!rightColOnly)
	const_for(S->BodyList, lljbody)
	{
		#define jbody (**lljbody)
		const_for(jbody.List, lobj)
		{
			*matrix->objectAtIndex(seg_eq_no, lobj->eq_no) = 1;
		}

		*matrix->objectAtIndex(seg_eq_no, jbody.eq_forces_no+0) = 0;
		*matrix->objectAtIndex(seg_eq_no, jbody.eq_forces_no+1) = 0;
		*matrix->objectAtIndex(seg_eq_no, jbody.eq_forces_no+2) = 2*(**lljbody).getArea();

		#undef jbody
	}

	//place solution pointer
	*matrix->solutionAtIndex(seg_eq_no) = &seg->g;

	//right column
	*matrix->rightColAtIndex(seg_eq_no) = -S->gsum() - S->InfCirculation;
}

void convectivefast::fillForceXEquation(TBody* ibody, bool rightColOnly)
{
	double _1_dt = 1/S->dt;
	int eq_no = ibody->eq_forces_no;
	if (!rightColOnly)
	const_for(S->BodyList, lljbody)
	{
		#define jbody (**lljbody)
		const_for(jbody.List, lobj)
		{
			if ( (ibody==*lljbody) && (ibody->kx >= 0) )
				*matrix->objectAtIndex(eq_no, lobj->eq_no) = _1_dt * (-lobj->corner.ry);
			else
				*matrix->objectAtIndex(eq_no, lobj->eq_no) = 0;
		}

		if (ibody == *lljbody)
		{
			if (ibody->kx >= 0)
			{
				*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) = -ibody->getArea()*_1_dt*ibody->density;
				*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = 0;
				*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = 0;
			} else
			{
				*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) = 1;
				*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = 0;
				*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = 0;
			}
		} else
		{
			*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) = 0;
			*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = 0;
			*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = 0;
		}

		#undef jbody
	}

	//place solution pointer
	*matrix->solutionAtIndex(eq_no) = &ibody->MotionSpeed_slae.rx;

	//right column
	if ( ibody->kx >= 0 )
		*matrix->rightColAtIndex(eq_no) = 
			- (ibody->density-1.0) * S->gravitation.rx * ibody->getArea()
			+ ibody->kx * ibody->deltaPosition.rx +
			- ibody->Friction_prev.rx
			+ ibody->Force_dead.rx
			- ibody->density * ibody->getArea() * ibody->MotionSpeed_slae_prev.rx * _1_dt
			- (-ibody->MotionSpeed_slae_prev.ry) * ibody->RotationSpeed_slae_prev * ibody->getArea()
			+ sqr(ibody->RotationSpeed_slae_prev) * ibody->getArea() * (ibody->getCom() - ibody->Position - ibody->deltaPosition).rx;
	else
		*matrix->rightColAtIndex(eq_no) = ibody->getSpeedX();
}

void convectivefast::fillForceYEquation(TBody* ibody, bool rightColOnly)
{
	double _1_dt = 1/S->dt;
	int eq_no = ibody->eq_forces_no+1;
	if (!rightColOnly)
	const_for(S->BodyList, lljbody)
	{
		#define jbody (**lljbody)
		const_for(jbody.List, lobj)
		{
			if ( (ibody==*lljbody) && (ibody->ky >= 0) )
				*matrix->objectAtIndex(eq_no, lobj->eq_no) = _1_dt * (lobj->corner.rx);
			else
				*matrix->objectAtIndex(eq_no, lobj->eq_no) = 0;
		}

		if (ibody == *lljbody)
		{
			if (ibody->ky >= 0)
			{
				*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) = 0;
				*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = -ibody->getArea()*_1_dt*ibody->density;
				*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = 0;
			} else
			{
				*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) = 0;
				*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = 1;
				*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = 0;
			}
		} else
		{
			*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) = 0;
			*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = 0;
			*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = 0;
		}
		#undef jbody
	}

	//place solution pointer
	*matrix->solutionAtIndex(eq_no) = &ibody->MotionSpeed_slae.ry;

	//right column
	if ( ibody->ky >= 0 )
		*matrix->rightColAtIndex(eq_no) = 
			- (ibody->density-1.0) * S->gravitation.ry * ibody->getArea()
			+ ibody->ky * ibody->deltaPosition.ry +
			- ibody->Friction_prev.ry
			+ ibody->Force_dead.ry
			- ibody->density * ibody->getArea() * ibody->MotionSpeed_slae_prev.ry * _1_dt
			- (ibody->MotionSpeed_slae_prev.rx) * ibody->RotationSpeed_slae_prev * ibody->getArea()
			+ sqr(ibody->RotationSpeed_slae_prev) * ibody->getArea() * (ibody->getCom() - ibody->Position - ibody->deltaPosition).ry;
	else
		*matrix->rightColAtIndex(eq_no) = ibody->getSpeedY();
}

void convectivefast::fillMomentEquation(TBody* ibody, bool rightColOnly)
{
	double _1_dt = 1/S->dt;
	double _1_2dt = 0.5/S->dt;
	int eq_no = ibody->eq_forces_no+2;
	TVec r_c_com = ibody->getCom() - ibody->Position - ibody->deltaPosition;

	if (!rightColOnly)
	const_for(S->BodyList, lljbody)
	{
		#define jbody (**lljbody)
		const_for(jbody.List, lobj)
		{
			if ( (ibody==*lljbody) && (ibody->ka >= 0) )
				*matrix->objectAtIndex(eq_no, lobj->eq_no) = _1_2dt * (lobj->corner - ibody->Position - ibody->deltaPosition).abs2();
			else
				*matrix->objectAtIndex(eq_no, lobj->eq_no) = 0;
		}

		if (ibody == *lljbody)
		{
			if (ibody->ka >= 0)
			{
				*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) = r_c_com.ry*ibody->getArea()*_1_dt;
				*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = -r_c_com.rx*ibody->getArea()*_1_dt;
				*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = -ibody->getMoi_c()*_1_dt*(ibody->density+2);
			} else
			{
				*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) = 0;
				*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = 0;
				*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = 1;
			}
		} else
		{
			*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) = 0;
			*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = 0;
			*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = 0;
		}
		#undef jbody
	}

	//place solution pointer
	*matrix->solutionAtIndex(eq_no) = &ibody->RotationSpeed_slae;

	//right column
	if ( ibody->ka >= 0 )
		*matrix->rightColAtIndex(eq_no) = 
			+ ibody->ka * ibody->deltaAngle +
			- (ibody->density-1.0) * (rotl(r_c_com)*S->gravitation) * ibody->getArea()
			+ ibody->Force_dead.g
			//- ibody->Friction_prev.g
			- rotl(r_c_com)*ibody->MotionSpeed_slae_prev * ibody->getArea() * _1_dt
			- ibody->getMoi_c() * ibody->RotationSpeed_slae_prev * _1_dt * (ibody->density + 2)
			- (r_c_com * ibody->MotionSpeed_slae_prev) * ibody->RotationSpeed_slae_prev * ibody->getArea();
	else
		*matrix->rightColAtIndex(eq_no) = ibody->getRotation();
}

void convectivefast::FillMatrix(bool rightColOnly)
{
	const_for(S->BodyList, llibody)
	{
		#pragma omp parallel for
		const_for((**llibody).List, latt)
		{
			bc::BoundaryCondition boundaryCondition = latt->bc;
			switch (boundaryCondition)
			{
				case bc::slip:
				case bc::noslip:
					fillSlipEquationForSegment(latt, rightColOnly);
					break;
				case bc::zero:
					fillZeroEquationForSegment(latt, rightColOnly);
					break;
				case bc::steady:
					fillSteadyEquationForSegment(latt, rightColOnly);
					break;
				case bc::inf_steady:
					fillInfSteadyEquationForSegment(latt, rightColOnly);
					break;
				default:
					fprintf(stderr, "Unknown bc: %d\n", boundaryCondition);
			}
		}

		fillForceXEquation(*llibody, rightColOnly);
		fillForceYEquation(*llibody, rightColOnly);
		fillMomentEquation(*llibody, rightColOnly);
	}

	if (!rightColOnly)
		matrix->markBodyMatrixAsFilled();
/*
	for (int i=0; i<matrix->size; i++)
	{
		for (int j=0; j< matrix->size; j++)
		{
			printf("%8.4lf\t", *matrix->objectAtIndex(i, j));
		}

		printf("\t%8.4lf\n", *matrix->rightColAtIndex(i));
	}
	printf("\n\n");*/
}

