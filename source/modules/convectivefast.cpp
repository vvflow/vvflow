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
		MatrixSize+=(**llbody).size()+9;
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
	const int seg_eq_no = seg->eq_no; //equation number for current segment

	//right column
	//influence of infinite speed
	*matrix->rightColAtIndex(seg_eq_no) = rotl(S->InfSpeed())*seg->dl;
	//influence of all free vortices
	TSortedNode* Node = S->Tree->findNode(seg->r);
	*matrix->rightColAtIndex(seg_eq_no) -= NodeInfluence(*Node, *seg);
	if (rightColOnly) return;

	//place solution pointer
	*matrix->solutionAtIndex(seg_eq_no) = &seg->g;

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
}

void convectivefast::fillZeroEquationForSegment(TAtt* seg, bool rightColOnly)
{
	const int seg_eq_no = seg->eq_no; //equation number for current segment

	//right column
	*matrix->rightColAtIndex(seg_eq_no) = 0;
	if (rightColOnly) return;

	//place solution pointer
	*matrix->solutionAtIndex(seg_eq_no) = &seg->g;
	
	//the only non-zero value
	*matrix->objectAtIndex(seg_eq_no, seg_eq_no) = 1;
}

void convectivefast::fillSteadyEquationForSegment(TAtt* seg, bool rightColOnly)
{
	const int seg_eq_no = seg->eq_no; //equation number for current segment
	TBody* lbody = seg->body;

	//right column
	*matrix->rightColAtIndex(seg_eq_no) = lbody->g_dead + 2*lbody->getArea()*lbody->Speed_slae_prev.o;
	lbody->g_dead = 0;
	if (rightColOnly) return;

	//place solution pointer
	*matrix->solutionAtIndex(seg_eq_no) = &seg->g;

	// self
	{
		#define jbody (*lbody)
		const_for(jbody.List, lobj)
			*matrix->objectAtIndex(seg_eq_no, lobj->eq_no) = 1;
		*matrix->objectAtIndex(seg_eq_no, jbody.eq_forces_no+2) = 2*lbody->getArea();
		#undef jbody
	}
}

void convectivefast::fillInfSteadyEquationForSegment(TAtt* seg, bool rightColOnly)
{
	const int seg_eq_no = seg->eq_no; //equation number for current segment

	//right column
	*matrix->rightColAtIndex(seg_eq_no) = -S->gsum() - S->InfCirculation;
	if (rightColOnly) return;
	
	//place solution pointer
	*matrix->solutionAtIndex(seg_eq_no) = &seg->g;

	// all
	const_for(S->BodyList, lljbody)
	{
		#define jbody (**lljbody)
		const_for(jbody.List, lobj)
			*matrix->objectAtIndex(seg_eq_no, lobj->eq_no) = 1;
		*matrix->objectAtIndex(seg_eq_no, jbody.eq_forces_no+2) = 2*(**lljbody).getArea();
		#undef jbody
	}

}

/*
888    888 Y88b   d88P 8888888b.  8888888b.   .d88888b.
888    888  Y88b d88P  888  "Y88b 888   Y88b d88P" "Y88b
888    888   Y88o88P   888    888 888    888 888     888
8888888888    Y888P    888    888 888   d88P 888     888
888    888     888     888    888 8888888P"  888     888
888    888     888     888    888 888 T88b   888     888
888    888     888     888  .d88P 888  T88b  Y88b. .d88P
888    888     888     8888888P"  888   T88b  "Y88888P"
*/

void convectivefast::fillHydroXEquation(TBody* ibody, bool rightColOnly)
{
	const int eq_no = ibody->eq_forces_no+3;
	const double _1_dt = 1/S->dt;
	const TVec r_c_com = ibody->getCom() - ibody->pos.r - ibody->dPos.r;

	//right column
	*matrix->rightColAtIndex(eq_no) = 
		+ ibody->getArea()*_1_dt*ibody->Speed_slae_prev.r.x
		+ ibody->getArea()*_1_dt*rotl(2*ibody->getCom()+r_c_com).x * ibody->Speed_slae_prev.o
		// - (-ibody->Speed_slae_prev.r.y) * ibody->Speed_slae_prev.o * ibody->getArea()
		+ sqr(ibody->Speed_slae_prev.o) * ibody->getArea() * r_c_com.x
		+ ibody->Force_dead.r.x;
	if (rightColOnly) return;

	//place solution pointer
	*matrix->solutionAtIndex(eq_no) = &ibody->Force_hydro.r.x;

	// self
	{
		#define jbody (*ibody)
		const_for(jbody.List, lobj)
			*matrix->objectAtIndex(eq_no, lobj->eq_no) = _1_dt * (-lobj->corner.y);

		// Speed_slae
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) = ibody->getArea()*_1_dt;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = 0;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = ibody->getArea()*_1_dt * (-2*ibody->getCom().y - r_c_com.y);

		// Force_hydro
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+3) = -1;
		#undef jbody
	}
}

void convectivefast::fillHydroYEquation(TBody* ibody, bool rightColOnly)
{
	const int eq_no = ibody->eq_forces_no+4;
	const double _1_dt = 1/S->dt;
	const TVec r_c_com = ibody->getCom() - ibody->pos.r - ibody->dPos.r;

	//right column
	*matrix->rightColAtIndex(eq_no) =
		+ ibody->getArea()*_1_dt*ibody->Speed_slae_prev.r.y
		+ ibody->getArea()*_1_dt*rotl(2*ibody->getCom()+r_c_com).y * ibody->Speed_slae_prev.o
		// - (ibody->Speed_slae_prev.r.x) * ibody->Speed_slae_prev.o * ibody->getArea()
		+ sqr(ibody->Speed_slae_prev.o) * ibody->getArea() * r_c_com.y
		+ ibody->Force_dead.r.y;
	if (rightColOnly) return;
	
	//place solution pointer
	*matrix->solutionAtIndex(eq_no) = &ibody->Force_hydro.r.y;

	// self
	{
		#define jbody (*ibody)
		const_for(jbody.List, lobj)
			*matrix->objectAtIndex(eq_no, lobj->eq_no) = _1_dt * (lobj->corner.x);

		// Speed_slae
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) = 0;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = ibody->getArea()*_1_dt;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = ibody->getArea()*_1_dt * (2*ibody->getCom().x + r_c_com.x);

		// Force_hydro
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+4) = -1;
		#undef jbody
	}
}

void convectivefast::fillHydroOEquation(TBody* ibody, bool rightColOnly)
{
	const int eq_no = ibody->eq_forces_no+5;
	const double _1_dt = 1/S->dt;
	const double _1_2dt = 0.5/S->dt;
	const TVec r_c_com = ibody->getCom() - ibody->pos.r - ibody->dPos.r;
	
	//right column
	*matrix->rightColAtIndex(eq_no) =
		+ ibody->getArea()*_1_dt * rotl(r_c_com) * ibody->Speed_slae_prev.r
		+ 2*ibody->getMoi_c()*_1_dt * ibody->Speed_slae_prev.o
		// - (r_c_com * ibody->Speed_slae_prev.r) * ibody->Speed_slae_prev.o * ibody->getArea()
		+ ibody->Force_dead.o;
	if (rightColOnly) return;
	
	//place solution pointer
	*matrix->solutionAtIndex(eq_no) = &ibody->Force_hydro.o;

	// self
	{
		#define jbody (*ibody)
		const_for(jbody.List, lobj)
			*matrix->objectAtIndex(eq_no, lobj->eq_no) = _1_2dt * (lobj->corner - ibody->pos.r - ibody->dPos.r).abs2();

		// Speed_slae
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) = ibody->getArea()*_1_dt * (-r_c_com.y);
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = ibody->getArea()*_1_dt * (r_c_com.x);
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = 2*ibody->getMoi_c()*_1_dt;

		// Force_hydro
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+5) = -1;
		#undef jbody
	}
}

/*
888b    888 8888888888 888       888 88888888888  .d88888b.  888b    888
8888b   888 888        888   o   888     888     d88P" "Y88b 8888b   888
88888b  888 888        888  d8b  888     888     888     888 88888b  888
888Y88b 888 8888888    888 d888b 888     888     888     888 888Y88b 888
888 Y88b888 888        888d88888b888     888     888     888 888 Y88b888
888  Y88888 888        88888P Y88888     888     888     888 888  Y88888
888   Y8888 888        8888P   Y8888     888     Y88b. .d88P 888   Y8888
888    Y888 8888888888 888P     Y888     888      "Y88888P"  888    Y888
*/

void convectivefast::fillNewtonXEquation(TBody* ibody, bool rightColOnly)
{
	const int eq_no = ibody->eq_forces_no+6;
	const double _1_dt = 1/S->dt;

	//right column
	*matrix->rightColAtIndex(eq_no) =
		- ibody->getArea()*_1_dt*ibody->density * ibody->Speed_slae_prev.r.x
		- (ibody->density-1.0) * S->gravitation.x * ibody->getArea()
		- ibody->Friction_prev.r.x;
	if (rightColOnly) return;
	
	//place solution pointer
	*matrix->solutionAtIndex(eq_no) = &ibody->Force_holder.r.x;

	// self
	{
		#define jbody (*ibody)

		// Speed_slae
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) = -ibody->getArea()*_1_dt*ibody->density;
		// Force_hydro
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+3) = 1;
		// Force_holder
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+6) = 1;
		#undef jbody
	}

	// children
	const_for(S->BodyList, lljbody)
	{
		#define jbody (**lljbody)
		if (jbody.root_body != ibody) continue;

		// Force_holder
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+6) = -1;
		#undef jbody
	}
}

void convectivefast::fillNewtonYEquation(TBody* ibody, bool rightColOnly)
{
	const int eq_no = ibody->eq_forces_no+7;
	const double _1_dt = 1/S->dt;

	//right column
	*matrix->rightColAtIndex(eq_no) =
		- ibody->getArea()*_1_dt*ibody->density * ibody->Speed_slae_prev.r.y
		- (ibody->density-1.0) * S->gravitation.y * ibody->getArea()
		- ibody->Friction_prev.r.y;
	if (rightColOnly) return;
	
	//place solution pointer
	*matrix->solutionAtIndex(eq_no) = &ibody->Force_holder.r.y;

	// self
	{
		#define jbody (*ibody)

		// Speed_slae
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = -ibody->getArea()*_1_dt*ibody->density;
		// Force_hydro
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+4) = 1;
		// Force_holder
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+7) = 1;
		#undef jbody
	}

	// children
	const_for(S->BodyList, lljbody)
	{
		#define jbody (**lljbody)
		if (jbody.root_body != ibody) continue;

		// Force_holder
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+7) = -1;
		#undef jbody
	}
}

void convectivefast::fillNewtonOEquation(TBody* ibody, bool rightColOnly)
{
	const int eq_no = ibody->eq_forces_no+8;
	const double _1_dt = 1/S->dt;
	const TVec r_c_com = ibody->getCom() - ibody->pos.r - ibody->dPos.r;

	//right column
	*matrix->rightColAtIndex(eq_no) =
		- ibody->getMoi_c()*_1_dt*ibody->density * ibody->Speed_slae_prev.o
		- (ibody->density-1.0) * (r_c_com*S->gravitation) * ibody->getArea()
		- ibody->Friction_prev.o;
	if (rightColOnly) return;

	//place solution pointer
	*matrix->solutionAtIndex(eq_no) = &ibody->Force_holder.o;

	// self
	{
		#define jbody (*ibody)
	
		// Speed_slae
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = -ibody->getMoi_c()*_1_dt*ibody->density;
		// Force_hydro
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+5) = 1;
		// Force_holder
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+8) = 1;
		#undef jbody
	}

	// children
	const_for(S->BodyList, lljbody)
	{
		#define jbody (**lljbody)
		if (jbody.root_body != ibody) continue;
		
		// Force_holder
		TVec r_child_c = jbody.pos.r + jbody.dPos.r - ibody->pos.r - ibody->dPos.r;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+6) = r_child_c.y;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+7) = -r_child_c.x;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+8) = -1;
		#undef jbody
	}
}

/*
888    888  .d88888b.   .d88888b.  888    d8P  8888888888
888    888 d88P" "Y88b d88P" "Y88b 888   d8P   888
888    888 888     888 888     888 888  d8P    888
8888888888 888     888 888     888 888d88K     8888888
888    888 888     888 888     888 8888888b    888
888    888 888     888 888     888 888  Y88b   888
888    888 Y88b. .d88P Y88b. .d88P 888   Y88b  888
888    888  "Y88888P"   "Y88888P"  888    Y88b 8888888888
*/

void convectivefast::fillHookeXEquation(TBody* ibody, bool rightColOnly)
{
	const int eq_no = ibody->eq_forces_no+0;

	*matrix->rightColAtIndex(eq_no) = -ibody->k.r.x * ibody->dPos.r.x;
	if (rightColOnly) return;

	//place solution pointer
	*matrix->solutionAtIndex(eq_no) = &ibody->Speed_slae.r.x;

	// self
	{
		#define jbody (*ibody)

		// Speed_slae
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) = jbody.damping.r.x;
		// Force_holder
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+6) = 1;
		#undef jbody
	}
}

void convectivefast::fillHookeYEquation(TBody* ibody, bool rightColOnly)
{
	const int eq_no = ibody->eq_forces_no+1;

	*matrix->rightColAtIndex(eq_no) = -ibody->k.r.y * ibody->dPos.r.y;
	if (rightColOnly) return;

	//place solution pointer
	*matrix->solutionAtIndex(eq_no) = &ibody->Speed_slae.r.y;

	// self
	{
		#define jbody (*ibody)

		// Speed_slae
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = jbody.damping.r.y;
		// Force_holder
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+7) = 1;
		#undef jbody
	}
}

void convectivefast::fillHookeOEquation(TBody* ibody, bool rightColOnly)
{
	const int eq_no = ibody->eq_forces_no+2;

	*matrix->rightColAtIndex(eq_no) = -ibody->k.o * ibody->dPos.o;
	if (rightColOnly) return;

	//place solution pointer
	*matrix->solutionAtIndex(eq_no) = &ibody->Speed_slae.o;

	// self
	{
		#define jbody (*ibody)

		// Speed_slae
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = jbody.damping.o;
		// Force_holder
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+8) = 1;
		#undef jbody
	}
}

/*
 .d8888b.  8888888b.  8888888888 8888888888 8888888b.
d88P  Y88b 888   Y88b 888        888        888  "Y88b
Y88b.      888    888 888        888        888    888
 "Y888b.   888   d88P 8888888    8888888    888    888
    "Y88b. 8888888P"  888        888        888    888
      "888 888        888        888        888    888
Y88b  d88P 888        888        888        888  .d88P
 "Y8888P"  888        8888888888 8888888888 8888888P"
*/

void convectivefast::fillSpeedXEquation(TBody* ibody, bool rightColOnly)
{
	const int eq_no = ibody->eq_forces_no+0;

	*matrix->rightColAtIndex(eq_no) = ibody->getSpeed().r.x;
	if (rightColOnly) return;

	//place solution pointer
	*matrix->solutionAtIndex(eq_no) = &ibody->Speed_slae.r.x;

	// self
	{
		#define jbody (*ibody)

		// Speed_slae
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) = 1;
		#undef jbody
	}

	// root
	if (ibody->root_body)
	{
		#define jbody (*ibody->root_body)
		
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) = -1;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = 0;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = (ibody->pos.r + ibody->dPos.r - jbody.pos.r - jbody.dPos.r).y;
		#undef jbody
	}
}

void convectivefast::fillSpeedYEquation(TBody* ibody, bool rightColOnly)
{
	const int eq_no = ibody->eq_forces_no+1;

	*matrix->rightColAtIndex(eq_no) = ibody->getSpeed().r.y;
	if (rightColOnly) return;

	//place solution pointer
	*matrix->solutionAtIndex(eq_no) = &ibody->Speed_slae.r.y;

	// self
	{
		#define jbody (*ibody)

		// Speed_slae
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = 1;
		#undef jbody
	}

	// root
	if (ibody->root_body)
	{
		#define jbody (*ibody->root_body)
		
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+0) = 0;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+1) = -1;
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = -(ibody->pos.r + ibody->dPos.r - jbody.pos.r - jbody.dPos.r).x;
		#undef jbody
	}
}

void convectivefast::fillSpeedOEquation(TBody* ibody, bool rightColOnly)
{
	const int eq_no = ibody->eq_forces_no+2;

	*matrix->rightColAtIndex(eq_no) = ibody->getSpeed().o;
	if (rightColOnly) return;

	//place solution pointer
	*matrix->solutionAtIndex(eq_no) = &ibody->Speed_slae.o;

	// self
	{
		#define jbody (*ibody)

		// Speed_slae
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = 1;
		#undef jbody
	}

	// root
	if (ibody->root_body)
	{
		#define jbody (*ibody->root_body)
		
		*matrix->objectAtIndex(eq_no, jbody.eq_forces_no+2) = -1;
		#undef jbody
	}
}

/*
888b     d888        d8888 88888888888 8888888b.  8888888 Y88b   d88P
8888b   d8888       d88888     888     888   Y88b   888    Y88b d88P
88888b.d88888      d88P888     888     888    888   888     Y88o88P
888Y88888P888     d88P 888     888     888   d88P   888      Y888P
888 Y888P 888    d88P  888     888     8888888P"    888      d888b
888  Y8P  888   d88P   888     888     888 T88b     888     d88888b
888   "   888  d8888888888     888     888  T88b    888    d88P Y88b
888       888 d88P     888     888     888   T88b 8888888 d88P   Y88b
*/

void convectivefast::FillMatrix(bool rightColOnly)
{
	if (!rightColOnly) matrix->fillWithZeros();

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

		fillHydroXEquation(*llibody, rightColOnly);
		fillHydroYEquation(*llibody, rightColOnly);
		fillHydroOEquation(*llibody, rightColOnly);

		fillNewtonXEquation(*llibody, rightColOnly);
		fillNewtonYEquation(*llibody, rightColOnly);
		fillNewtonOEquation(*llibody, rightColOnly);		

		if ((*llibody)->k.r.x >= 0 && S->Time>0)
			fillHookeXEquation(*llibody, rightColOnly);
		else
			fillSpeedXEquation(*llibody, rightColOnly);

		if ((*llibody)->k.r.y >= 0 && S->Time>0)
			fillHookeYEquation(*llibody, rightColOnly);
		else
			fillSpeedYEquation(*llibody, rightColOnly);

		if ((*llibody)->k.o >= 0 && S->Time>0)
			fillHookeOEquation(*llibody, rightColOnly);
		else
			fillSpeedOEquation(*llibody, rightColOnly);
	}

	if (!rightColOnly)
		matrix->markBodyMatrixAsFilled();

	// matrix->save("matrix");
}

