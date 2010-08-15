	program filtr1
	IMPLICIT REAL*8 (A-H,O-Z)
	dimension x(10000),y(10000),y1(10000)
	open(50,file='xy.dat')
	open(51,file='res.dat')
	read(50,*) n,k1
	do i=1,n
	read (50,*) x(i),y(i)
	enddo
	xmin=x(1)
	xmax=x(1)
	do i=1,n
	if(x(i).lt.xmin) xmin=x(i)
	if(x(i).gt.xmax) xmax=x(i)
	enddo
	call filtr(X,Y,n,k1,y1,xmin,xmax)
	do i=1,n
	write(51,*) x(i),y1(i)
	enddo
	end
	subroutine filtr(X,Y,n,k1,y1,xmin,xmax)
	IMPLICIT REAL*8 (A-H,O-Z)
	real*8 X(n),Y(n),y1(n)
	pi2=3.14159*k1/2.
	sigm=pi2/(xmax-xmin)
	dl=(xmax-xmin)/k1
	y1=0.
	do i=1,k1+1
		xmint=xmin+(i-2)*dl
		xmaxt=xmin+i*dl
		c1=0.
		c2=0.
		c3=0.
		c4=0.
			nk=0
		do j=1,n
			if(x(j).ge.xmint.and.x(j).le.xmaxt)then
				c1=c1+x(j)*x(j)
				c2=c2+x(j)
				c3=c3+y(j)
				c4=c4+x(j)*y(j)
				nk=nk+1
			endif
		enddo
		a=(nk*c4-c2*c3)/(nk*c1-c2*c2)
		b=(c4-a*c1)/c2
		x0=xmin+(i-1)*dl
		do j=1,n
			if(x(j).ge.xmint.and.x(j).le.xmaxt) y1(j)=y1(j)+(a*x(j)+b)*(cos(sigm*(x(j)-x0)))**2
		enddo
	enddo
	return
	end subroutine filtr
