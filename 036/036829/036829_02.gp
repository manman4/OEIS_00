\\ G.f.: (g-1)/(3-2*g)^2 where g=1+x*g^3.

my(N=30, x='x+O('x^N), g=sum(k=0, N, binomial(3*k, k)/(2*k+1)*x^k)); 1+x*g^3 - g
my(N=30, x='x+O('x^N), g=sum(k=0, N, binomial(3*k, k)/(2*k+1)*x^k)); concat(0, Vec((g-1)/(3-2*g)^2))
my(N=30, x='x+O('x^N), g=sum(k=0, N, binomial(3*k, k)/(2*k+1)*x^k)); (g-1)/(3-2*g)^2