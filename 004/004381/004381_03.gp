\\ G.f.: g/(8-7*g) where g = 1+x*g^8 is the g.f. of A007556. 
my(N=30, x='x+O('x^N), g=sum(k=0, N, binomial(8*k, k)/(7*k+1)*x^k)); Vec( g )
my(N=30, x='x+O('x^N), g=sum(k=0, N, binomial(8*k, k)/(7*k+1)*x^k)); Vec( 1+x*g^8 - g )
my(N=30, x='x+O('x^N), g=sum(k=0, N, binomial(8*k, k)/(7*k+1)*x^k)); Vec( g/(8-7*g) )