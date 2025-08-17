\\ G.f.: 1/(1 - x*g^2*(6-2*g)) where g = 1+x*g^3 is the g.f. of A001764.
my(N=30, x='x+O('x^N), g=sum(k=0, N, binomial(3*k, k)/(2*k+1)*x^k)); Vec( g )
my(N=30, x='x+O('x^N), g=sum(k=0, N, binomial(3*k, k)/(2*k+1)*x^k)); Vec( 1+x*g^3 - g )
my(N=30, x='x+O('x^N), g=sum(k=0, N, binomial(3*k, k)/(2*k+1)*x^k)); Vec( 1/(1 - x*g^2*(6-2*g)) )

\\ G.f.: g/((2-g) * (3-2*g)) where g = 1+x*g^3 is the g.f. of A001764.
my(N=30, x='x+O('x^N), g=sum(k=0, N, binomial(3*k, k)/(2*k+1)*x^k)); Vec( 1/(1 - x*g^2*(6-2*g)) - g/((2-g)*(3-2*g)) )
