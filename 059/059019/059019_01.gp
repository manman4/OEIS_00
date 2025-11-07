M=20;

\\ G.f.: g/(1+x^3*g^4) where g = 1+x*g^2 is the g.f. of A000108.
my(N=M, x='x+O('x^N), g=sum(k=0, N, binomial(2*k, k)/(k+1)*x^k)); Vec( g )
my(N=M, x='x+O('x^N), g=sum(k=0, N, binomial(2*k, k)/(k+1)*x^k)); Vec( 1+x*g^2-g )
my(N=M, x='x+O('x^N), g=sum(k=0, N, binomial(2*k, k)/(k+1)*x^k)); Vec( g/(1-x^3*g^4) )

