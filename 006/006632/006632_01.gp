M=20;

\\ G.f.: x*g^3 where g = 1+x*g^4 is the g.f. of A002293.
my(N=M, x='x+O('x^N), g=sum(k=0, N, binomial(4*k, k)/(3*k+1)*x^k)); Vec( g ) 
my(N=M, x='x+O('x^N), g=sum(k=0, N, binomial(4*k, k)/(3*k+1)*x^k)); Vec( 1+x*g^4 - g ) 
my(N=M, x='x+O('x^N), g=sum(k=0, N, binomial(4*k, k)/(3*k+1)*x^k)); Vec( x*g^3 ) 

