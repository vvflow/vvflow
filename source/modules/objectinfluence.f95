subroutine FortObjectInfluence(x,y,x1,y1,x2,y2,ax,ay,eps) 
IMPLICIT REAL*8 (A-H,O-Z)
complex*16 z,z1,z2,z3,a
z=dcmplx(x,-y)
z1=dcmplx(x1,-y1)
z2=dcmplx(x2,-y2)
c1=cdabs(z-z1)
c2=cdabs(z-z2)
if(c1.ge.eps.and.c2.ge.eps) then
a=-cdlog((z-z1)/(z-z2))/(z2-z1)*cdabs(z2-z1)
ay=dreal(a)
ax=-dimag(a)
else
if(c1.le.eps.and.c2.le.eps) then
a=-(z-(z2+z1)*0.5)*cdabs(z2-z1)/eps**2
ay=dreal(a)
ax=dimag(a)
else
a0=(x2-x1)**2+(y2-y1)**2
b0=(x-x1)*(x2-x1)+(y-y1)*(y2-y1)
c0=(x-x1)**2+(y-y1)**2-eps**2
d=sqrt(b0*b0-a0*c0)
alf=(b0+d)/a0
if(alf.gt.1..or.alf.lt.0.) alf=(b0-d)/a0
z3=z1+alf*(z2-z1)
if(c1.lt.eps.and.c2.ge.eps) then
a=-cdlog((z-z3)/(z-z2))/(z2-z3)*cdabs(z2-z3)
ay=dreal(a)
ax=-dimag(a)
a=-(z-(z3+z1)*0.5)*cdabs(z3-z1)/eps**2
ay=dreal(a)+ay
ax=dimag(a)+ax
endif
if(c1.ge.eps.and.c2.lt.eps) then
a=-cdlog((z-z1)/(z-z3))/(z3-z1)*cdabs(z3-z1)
ay=dreal(a)
ax=-dimag(a)
a=-(z-(z2+z3)*0.5)*cdabs(z2-z3)/eps**2
ay=dreal(a)+ay
ax=dimag(a)+ax
endif
endif
endif
end subroutine FortObjectInfluence

