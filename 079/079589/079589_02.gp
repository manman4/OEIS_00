\\ G.f.: 1/(1 - x*g^3*(5+g)) where g = 1+x*g^5 is the g.f. of A002294.
my(N=30, x='x+O('x^N), g=sum(k=0, N, binomial(5*k, k)/(4*k+1)*x^k)); Vec( g )
my(N=30, x='x+O('x^N), g=sum(k=0, N, binomial(5*k, k)/(4*k+1)*x^k)); Vec( 1+x*g^5 - g )
my(N=30, x='x+O('x^N), g=sum(k=0, N, binomial(5*k, k)/(4*k+1)*x^k)); Vec( 1/(1 - x*g^3*(5+g)) )