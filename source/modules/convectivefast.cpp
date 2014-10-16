#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <assert.h>
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
	TVec dr = p - obj.r;
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

			TVec DistP = TVec(bnode.x, bnode.y) - fnode.CMp.r;
			TVec DistM = TVec(bnode.x, bnode.y) - fnode.CMm.r;

//			double _1_DistPabs = 1./DistP.abs2();
//			double _1_DistMabs = 1./DistM.abs2();
			double FuncP1 = fnode.CMp.g / DistP.abs2(); //Extremely complicated useless variables
			double FuncM1 = fnode.CMm.g / DistM.abs2();
			double FuncP2 = fnode.CMp.g / sqr(DistP.abs2());
			double FuncM2 = fnode.CMm.g / sqr(DistM.abs2());
			#undef fnode

			Teilor1 -= (FuncP1*DistP.y + FuncM1*DistM.y);
			Teilor2 += (FuncP1*DistP.x + FuncM1*DistM.x);
			Teilor3 += (FuncP2*DistP.y*DistP.x + FuncM2*DistM.y*DistM.x);
			Teilor4 += (FuncP2 * (sqr(DistP.y) - sqr(DistP.x)) + FuncM2 * (sqr(DistM.y) - sqr(DistM.x)));
		}

		Teilor1 *= C_1_2PI;
		Teilor2 *= C_1_2PI;
		Teilor3 *= C_1_PI;
		Teilor4 *= C_1_2PI;

		TVec dr_local, nodeCenter(bnode.x, bnode.y);
		
		for (TObj *lobj = bnode.vRange.first; lobj < bnode.vRange.last; lobj++)
		{
			if (!lobj->g) {continue;}
			dr_local = lobj->r - nodeCenter;
			lobj->v += TVec(Teilor1, Teilor2) + S->InfSpeed() + SpeedSum(bnode, lobj->r) +
			         TVec(TVec(Teilor3,  Teilor4)*dr_local,
			              TVec(Teilor4, -Teilor3)*dr_local);
		}

		for (TObj *lobj = bnode.hRange.first; lobj < bnode.hRange.last; lobj++)
		{
			if (!lobj->g) {continue;}
			dr_local = lobj->r - nodeCenter;
			lobj->v += TVec(Teilor1, Teilor2) + S->InfSpeed() + SpeedSum(bnode, lobj->r) +
			         TVec(TVec(Teilor3,  Teilor4)*dr_local,
			              TVec(Teilor4, -Teilor3)*dr_local);
		}

		for (TObj *lobj = bnode.sRange.first; lobj < bnode.sRange.last; lobj++)
		{
			dr_local = lobj->r - nodeCenter;
			lobj->v += TVec(Teilor1, Teilor2) + S->InfSpeed() + SpeedSum(bnode, lobj->r) +
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
		bool calcBC = !(**llbody).Speed_slae.iszero();
		bool calcBCS = false;
		const_for((**llbody).List, latt) { if (latt->bc == bc::slip) {calcBCS = true; break;}}

		if (!calcBC && !calcBCS) continue;

		if (S->VortexList)
		{
			#pragma omp parallel for
			const_for(S->VortexList, lobj)
			{
				if (calcBC) lobj->v += BoundaryConvective(**llbody, lobj->r)*C_1_2PI;
				if (calcBCS) lobj->v += BoundaryConvectiveSlip(**llbody, lobj->r)*C_1_2PI;
			}
		}

		if (S->HeatList)
		{
			#pragma omp parallel for
			const_for(S->HeatList, lobj)
			{
				if (calcBC) lobj->v += BoundaryConvective(**llbody, lobj->r)*C_1_2PI;
				if (calcBCS) lobj->v += BoundaryConvectiveSlip(**llbody, lobj->r)*C_1_2PI;
			}
		}

		if (S->StreakList)
		{
			#pragma omp parallel for
			const_for(S->StreakList, lobj)
			{
				if (calcBC) lobj->v += BoundaryConvective(**llbody, lobj->r)*C_1_2PI;
				if (calcBCS) lobj->v += BoundaryConvectiveSlip(**llbody, lobj->r)*C_1_2PI;
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
		double drabs2 = (p-latt->r).abs2();
		if (drabs2 < latt->dl.abs2())
		{
			TVec Vs1 = b.Speed_slae.r + b.Speed_slae.o * rotl(latt->corner - (b.pos.r + b.dPos.r));
			double g1 = -Vs1 * latt->dl;
			double q1 = -rotl(Vs1) * latt->dl; 

			TVec Vs2 = b.Speed_slae.r + b.Speed_slae.o * rotl(latt->corner + latt->dl - (b.pos.r + b.dPos.r));
			double g2 = -Vs2 * latt->dl;
			double q2 = -rotl(Vs2) * latt->dl;

			res+= (rotl(SegmentInfluence_linear_source(p, *latt, g1, g2)) + SegmentInfluence_linear_source(p, *latt, q1, q2));
		} else
		{
			TVec Vs = b.Speed_slae.r + b.Speed_slae.o * rotl(latt->r - (b.pos.r + b.dPos.r));
			double g = -Vs * latt->dl;
			double q = -rotl(Vs) * latt->dl; 
			TVec dr = p - latt->r;
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

		if (lbody1->k.r.x >= 0) return false;
		if (lbody1->k.r.y >= 0) return false;
		if (lbody1->k.o >= 0) return false;

		const_for(S->BodyList, llbody2)
		{
			TBody *lbody2 = *llbody2;
			if ((lbody1->getSpeed().o!=0) || (lbody2->getSpeed().o!=0))
			{
				if (lbody1->getSpeed().o != lbody2->getSpeed().o) return false;
				if ((lbody1->pos.r+lbody1->dPos.r) != (lbody2->pos.r+lbody2->dPos.r)) return false;
			}

			if (lbody1->getSpeed().r != lbody2->getSpeed().r) return false;
		}
	}

	return true;
}

void convectivefast::CalcCirculationFast()
{
	bool use_inverse = canUseInverse() && S->Time>0;

	if (matrix->bodyMatrixIsOk())
		FillMatrix(true);
	else
		FillMatrix(false);

	matrix->solveUsingInverseMatrix(use_inverse);
}

double convectivefast::_2PI_Xi_g(TVec p, const TAtt &seg, double rd) // in doc 2\pi\Xi_\gamma (1.7)
{
	complex<double> z(p.x, -p.y);
	complex<double> zc(seg.r.x,-seg.r.y);
	complex<double> dz(seg.dl.x,-seg.dl.y);
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
		return ((seg.r-p)*seg.dl)/(rd*rd);
	}
	else
	{
		double a0 = seg.dl.abs2();
		double b0 = (p-seg.r)*seg.dl;
		double d  = sqrt(b0*b0-a0*((p-seg.r).abs2()-rd*rd));
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
	if (&p == &seg.r) { TVec pnew = p+rotl(seg.dl)*0.001; return _2PI_Xi_q(pnew, seg, rd); }
	complex<double> z(p.x, -p.y);
	complex<double> zc(seg.r.x,-seg.r.y);
	complex<double> dz(seg.dl.x,-seg.dl.y);
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
		return ((seg.r-p)*rotl(seg.dl))/(rd*rd);
	}
	else
	{
		double a0 = seg.dl.abs2();
		double b0 = (p-seg.r)*seg.dl;
		double d  = sqrt(b0*b0-a0*((p-seg.r).abs2()-rd*rd));
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
	*_2PI_A1 = (seg.body == &b)?  C_2PI * seg.dl.y : 0;
	*_2PI_A2 = (seg.body == &b)? -C_2PI * seg.dl.x : 0;
	*_2PI_A3 = 0;
	if ((b.k.o<0) && (!b.getSpeed().o)) 
	{
		//FIXME econome time. uncomment return
		//fprintf(stderr, "ret:\t%lf\t%lf\n", seg.corner.rx, seg.corner.ry);
		return;
	}
	const_for(b.List, latt)
	{
		double _2piXi_g = _2PI_Xi_g(latt->r, seg, latt->dl.abs()*0.25);
		double _2piXi_q = _2PI_Xi_q(latt->r, seg, latt->dl.abs()*0.25);
		TVec Xi(_2piXi_g, _2piXi_q);
		TVec r0 = latt->r - (b.pos.r + b.dPos.r);
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
			res+= _2PI_Xi_g(lobj->r, seg, 1./lobj->_1_eps) * lobj->g;
		}
	}

	const_for(Node.FarNodes, llfnode)
	{
		#define fnode (**llfnode)
		res+= _2PI_Xi_g(fnode.CMp.r, seg, 0) * fnode.CMp.g;
		res+= _2PI_Xi_g(fnode.CMm.r, seg, 0) * fnode.CMm.g;
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
			TVec Vs = body.Speed_slae.r + body.Speed_slae.o * rotl(latt->r - (body.pos.r + body.dPos.r));
			if (latt == &seg) { res_tmp+= -(-rotl(Vs) * latt->dl)*0.5*C_2PI; continue; }
			res+= _2PI_Xi_g(latt->r, seg, rd) * (-Vs * latt->dl);
			res+= _2PI_Xi_q(latt->r, seg, rd) * (-rotl(Vs) * latt->dl);
		}
		#undef body
	}

	return res * C_1_2PI;
}

TVec convectivefast::SegmentInfluence_linear_source(TVec p, const TAtt &seg, double q1, double q2)
{
//	cerr << "orly?" << endl;
	complex<double> z(p.x, p.y);
	complex<double> zc(seg.r.x, seg.r.y);
	complex<double> dz(seg.dl.x, seg.dl.y);
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
			*matrix->objectAtIndex(seg_eq_no, lobj->eq_no) = _2PI_Xi_g(lobj->corner, *seg, lobj->dl.abs()*0.25)*C_1_2PI;
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
	TSortedNode* Node = S->Tree->findNode(seg->r);
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
	*matrix->rightColAtIndex(seg_eq_no) = lbody->g_dead + 2*lbody->getArea()*lbody->Speed_slae_prev.o;
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
	const double _1_dt = 1/S->dt;
	const int eq_no = ibody->eq_forces_no;

	if (!rightColOnly)
	// свое тело
	{
		#define jbody (*ibody)
		const_for(jbody.List, lobj)
			*matrix->objectAtIndex(eq_no, lobj->eq_no) = _1_dt * (-lobj->corner.y);

		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) = -ibody->getArea()*_1_dt*ibody->density
		                                                      -ibody->damping.r.x;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = 0;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = 0;
		#undef jbody
	}

	if (!rightColOnly && ibody->root_body)
	// опорное тело
	{
		#define jbody (*ibody->root_body)
		const_for(jbody.List, lobj)
			*matrix->objectAtIndex(eq_no, lobj->eq_no) = 0;
		
		fprintf(stderr, "fillForceXEquation: root_body not implemented yet\n");
		#undef jbody
	}

	if (!rightColOnly)
	const_for(S->BodyList, lljbody) // чужие тела
	{
		if (*lljbody == ibody) continue;
		if (*lljbody == ibody->root_body) continue;

		#define jbody (**lljbody)
		const_for(jbody.List, lobj)
			*matrix->objectAtIndex(eq_no, lobj->eq_no) = 0;

		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) = 0;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = 0;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = 0;

		#undef jbody
	}

	//place solution pointer
	*matrix->solutionAtIndex(eq_no) = &ibody->Speed_slae.r.x;

	//right column
	*matrix->rightColAtIndex(eq_no) = 
		- (ibody->density-1.0) * S->gravitation.x * ibody->getArea()
		+ ibody->k.r.x * ibody->dPos.r.x +
		- ibody->Friction_prev.r.x
		+ ibody->Force_dead.r.x
		- ibody->density * ibody->getArea() * ibody->Speed_slae_prev.r.x * _1_dt
		- (-ibody->Speed_slae_prev.r.y) * ibody->Speed_slae_prev.o * ibody->getArea()
		+ sqr(ibody->Speed_slae_prev.o) * ibody->getArea() * (ibody->getCom() - ibody->pos.r - ibody->dPos.r).x;
}

void convectivefast::fillForceYEquation(TBody* ibody, bool rightColOnly)
{
	const double _1_dt = 1/S->dt;
	const int eq_no = ibody->eq_forces_no+1;

	if (!rightColOnly)
	// свое тело
	{
		#define jbody (*ibody)
		const_for(jbody.List, lobj)
			*matrix->objectAtIndex(eq_no, lobj->eq_no) = _1_dt * (lobj->corner.x);
		
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) = 0;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = -ibody->getArea()*_1_dt*ibody->density
		                                                      -ibody->damping.r.y;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = 0;

		#undef jbody
	}

	if (!rightColOnly && ibody->root_body)
	// опорное тело
	{
		#define jbody (*ibody->root_body)
		const_for(jbody.List, lobj)
			*matrix->objectAtIndex(eq_no, lobj->eq_no) = 0;

		fprintf(stderr, "fillForceYEquation: root_body not implemented yet\n");
		#undef jbody
	}

	if (!rightColOnly)
	const_for(S->BodyList, lljbody) // чужие тела
	{
		if (*lljbody == ibody) continue;
		if (*lljbody == ibody->root_body) continue;

		#define jbody (**lljbody)
		const_for(jbody.List, lobj)
			*matrix->objectAtIndex(eq_no, lobj->eq_no) = 0;

		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) = 0;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = 0;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = 0;
		#undef jbody
	}

	//place solution pointer
	*matrix->solutionAtIndex(eq_no) = &ibody->Speed_slae.r.y;

	//right column
	*matrix->rightColAtIndex(eq_no) = 
		- (ibody->density-1.0) * S->gravitation.y * ibody->getArea()
		+ ibody->k.r.y * ibody->dPos.r.y +
		- ibody->Friction_prev.r.y
		+ ibody->Force_dead.r.y
		- ibody->density * ibody->getArea() * ibody->Speed_slae_prev.r.y * _1_dt
		- (ibody->Speed_slae_prev.r.x) * ibody->Speed_slae_prev.o * ibody->getArea()
		+ sqr(ibody->Speed_slae_prev.o) * ibody->getArea() * (ibody->getCom() - ibody->pos.r - ibody->dPos.r).y;
}

void convectivefast::fillForceOEquation(TBody* ibody, bool rightColOnly)
{
	const double _1_dt = 1/S->dt;
	const double _1_2dt = 0.5/S->dt;
	const int eq_no = ibody->eq_forces_no+2;
	const TVec r_c_com = ibody->getCom() - ibody->pos.r - ibody->dPos.r;

	if (!rightColOnly)
	// свое тело
	{
		#define jbody (*ibody)
		if (ibody->k.o >= 0 && S->Time>0)
		const_for(jbody.List, lobj)
			*matrix->objectAtIndex(eq_no, lobj->eq_no) = _1_2dt * (lobj->corner - ibody->pos.r - ibody->dPos.r).abs2();

		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) =  r_c_com.y*ibody->getArea()*_1_dt;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = -r_c_com.x*ibody->getArea()*_1_dt;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = -ibody->getMoi_c()*_1_dt*(ibody->density+2)
		                                                      -ibody->damping.o;
		if (ibody->root_body)
		{
			*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) *= (ibody->density+1);
			*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) *= (ibody->density+1);
		}
		#undef jbody
	}

	if (!rightColOnly && ibody->root_body)
	// опорное тело
	{
		#define jbody (*ibody->root_body)

		const_for(jbody.List, lobj)
			*matrix->objectAtIndex(eq_no, lobj->eq_no) = 0;
		
		fprintf(stderr, "fillForceOEquation: root_body not implemented yet\n");
		// TVec r_c_root = ibody->pos.r + ibody->dPos.r - jbody.pos.r - jbody.dPos.r;
		// *matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) = r_c_com.x*ibody->getArea()*_1_dt;
		// *matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = - r_c_com.y*ibody->getArea()*_1_dt;
		// *matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = r_c_com * rotl(r_c_root)*ibody->getArea()*_1_dt;
		#undef jbody
	}

	if (!rightColOnly)
	const_for(S->BodyList, lljbody) //чужие тела
	{
		if (*lljbody == ibody) continue;
		if (*lljbody == ibody->root_body) continue;
		#define jbody (**lljbody)

		const_for(jbody.List, lobj)
			*matrix->objectAtIndex(eq_no, lobj->eq_no) = 0;

		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) = 0;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = 0;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = 0;

		#undef jbody
	}

	//place solution pointer
	*matrix->solutionAtIndex(eq_no) = &ibody->Speed_slae.o;

	//right column
	*matrix->rightColAtIndex(eq_no) = 
		+ ibody->k.o * ibody->dPos.o
		- (ibody->density-1.0) * (rotl(r_c_com)*S->gravitation) * ibody->getArea()
		+ ibody->Force_dead.o
		- ibody->Friction_prev.o
		- rotl(r_c_com)*ibody->Speed_slae_prev.r * ibody->getArea() * _1_dt
		- ibody->getMoi_c() * ibody->Speed_slae_prev.o * _1_dt * (ibody->density + 2.)
		- (r_c_com * ibody->Speed_slae_prev.r) * ibody->Speed_slae_prev.o * ibody->getArea();
	if (ibody->root_body)
	{
		*matrix->rightColAtIndex(eq_no) +=
		 - ibody->density * (rotl(r_c_com)*ibody->Speed_slae_prev.r) * ibody->getArea() * _1_dt;
	}
}

void convectivefast::fillSpeedXEquation(TBody* ibody, bool rightColOnly)
{
	const double _1_dt = 1/S->dt;
	const int eq_no = ibody->eq_forces_no;

	if (!rightColOnly)
	// свое тело
	{
		#define jbody (*ibody)
		const_for(jbody.List, lobj)
			*matrix->objectAtIndex(eq_no, lobj->eq_no) = 0;

		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) = 1;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = 0;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = 0;
		#undef jbody
	}

	if (!rightColOnly && ibody->root_body)
	// опорное тело
	{
		#define jbody (*ibody->root_body)
		const_for(jbody.List, lobj)
			*matrix->objectAtIndex(eq_no, lobj->eq_no) = 0;

		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) = -1;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = 0;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = (ibody->pos.r + ibody->dPos.r - jbody.pos.r - jbody.dPos.r).y;
		#undef jbody
	}

	if (!rightColOnly)
	const_for(S->BodyList, lljbody) // чужие тела
	{
		if (*lljbody == ibody) continue;
		if (*lljbody == ibody->root_body) continue;

		#define jbody (**lljbody)
		const_for(jbody.List, lobj)
			*matrix->objectAtIndex(eq_no, lobj->eq_no) = 0;

		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) = 0;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = 0;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = 0;
		#undef jbody
	}

	//place solution pointer
	*matrix->solutionAtIndex(eq_no) = &ibody->Speed_slae.r.x;

	*matrix->rightColAtIndex(eq_no) = ibody->getSpeed().r.x;
}

void convectivefast::fillSpeedYEquation(TBody* ibody, bool rightColOnly)
{
	const double _1_dt = 1/S->dt;
	const int eq_no = ibody->eq_forces_no+1;

	if (!rightColOnly)
	// свое тело
	{
		#define jbody (*ibody)
		const_for(jbody.List, lobj)
			*matrix->objectAtIndex(eq_no, lobj->eq_no) = 0;
		
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) = 0;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = 1;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = 0;

		#undef jbody
	}

	if (!rightColOnly && ibody->root_body)
	// опорное тело
	{
		#define jbody (*ibody->root_body)
		const_for(jbody.List, lobj)
			*matrix->objectAtIndex(eq_no, lobj->eq_no) = 0;

		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) = 0;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = -1;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = -(ibody->pos.r + ibody->dPos.r - jbody.pos.r - jbody.dPos.r).x;
		#undef jbody
	}

	if (!rightColOnly)
	const_for(S->BodyList, lljbody) // чужие тела
	{
		if (*lljbody == ibody) continue;
		if (*lljbody == ibody->root_body) continue;

		#define jbody (**lljbody)
		const_for(jbody.List, lobj)
			*matrix->objectAtIndex(eq_no, lobj->eq_no) = 0;

		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) = 0;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = 0;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = 0;
		#undef jbody
	}

	//place solution pointer
	*matrix->solutionAtIndex(eq_no) = &ibody->Speed_slae.r.y;

	*matrix->rightColAtIndex(eq_no) = ibody->getSpeed().r.y;
}

void convectivefast::fillSpeedOEquation(TBody* ibody, bool rightColOnly)
{
	const double _1_dt = 1/S->dt;
	const double _1_2dt = 0.5/S->dt;
	const int eq_no = ibody->eq_forces_no+2;
	const TVec r_c_com = ibody->getCom() - ibody->pos.r - ibody->dPos.r;

	if (!rightColOnly)
	// свое тело
	{
		#define jbody (*ibody)
		const_for(jbody.List, lobj)
			*matrix->objectAtIndex(eq_no, lobj->eq_no) = 0;
		
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) = 0;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = 0;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = 1;

		#undef jbody
	}

	if (!rightColOnly && ibody->root_body)
	// опорное тело
	{
		#define jbody (*ibody->root_body)
		const_for(jbody.List, lobj)
			*matrix->objectAtIndex(eq_no, lobj->eq_no) = 0;

		fprintf(stderr, "fillSpeedOEquation: root_body not implemented yet\n");
		#undef jbody
	}

	if (!rightColOnly)
	const_for(S->BodyList, lljbody) // чужие тела
	{
		if (*lljbody == ibody) continue;
		if (*lljbody == ibody->root_body) continue;

		#define jbody (**lljbody)
		const_for(jbody.List, lobj)
			*matrix->objectAtIndex(eq_no, lobj->eq_no) = 0;

		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) = 0;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = 0;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = 0;
		#undef jbody
	}

	//place solution pointer
	*matrix->solutionAtIndex(eq_no) = &ibody->Speed_slae.o;

	//right column
	*matrix->rightColAtIndex(eq_no) = ibody->getSpeed().o;
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

		if ((*llibody)->k.r.x >= 0 && S->Time>0)
			fillForceXEquation(*llibody, rightColOnly);
		else
			fillSpeedXEquation(*llibody, rightColOnly);

		if ((*llibody)->k.r.y >= 0 && S->Time>0)
			fillForceYEquation(*llibody, rightColOnly);
		else
			fillSpeedYEquation(*llibody, rightColOnly);

		if ((*llibody)->k.o >= 0 && S->Time>0)
			fillForceOEquation(*llibody, rightColOnly);
		else
			fillSpeedOEquation(*llibody, rightColOnly);
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

