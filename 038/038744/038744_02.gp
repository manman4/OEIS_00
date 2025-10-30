\\ G.f.: (g-1)/((3*g-1)*(g^2-3*g+1)) where g*(1-g)^2 = x. 
my(N=30, x='x+O('x^N), f=sum(k=0, N, binomial(3*k, k)/(2*k+1)*x^k), g=x*f^2); Vec( g*(1-g)^2 - x )
my(N=30, x='x+O('x^N), f=sum(k=0, N, binomial(3*k, k)/(2*k+1)*x^k), g=x*f^2); Vec( (g-1)/((3*g-1)*(g^2-3*g+1)) )

\\ G.f.: g/((1-3*x*g^2) * (1-x*g^4)) where g = 1+x*g^3 is the g.f. of A001764.
my(N=30, x='x+O('x^N), g=sum(k=0, N, binomial(3*k, k)/(2*k+1)*x^k)); Vec( g )
my(N=30, x='x+O('x^N), g=sum(k=0, N, binomial(3*k, k)/(2*k+1)*x^k)); Vec( 1+x*g^3 - g )
my(N=30, x='x+O('x^N), g=sum(k=0, N, binomial(3*k, k)/(2*k+1)*x^k)); Vec( g/((1-3*x*g^2)*(1-x*g^4)) )