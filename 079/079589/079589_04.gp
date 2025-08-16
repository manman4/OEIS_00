\\ G.f.: B(x)^2/(1 + 4*(B(x)-1)/5), where B(x) is the g.f. of A001449. 
my(N=30, x='x+O('x^N), b=sum(k=0, N, binomial(5*k, k)*x^k)); Vec( b )
my(N=30, x='x+O('x^N), b=sum(k=0, N, binomial(5*k, k)*x^k)); Vec( b^2/(1 + 4*(b-1)/5) )