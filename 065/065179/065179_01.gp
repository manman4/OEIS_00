\\ G.f.: Sum_{k>=1} mu(k) * log(1 + x^k/(1 - 4*x^k))/k.
my(M=40, x='x+O('x^M)); Vec(sum(k=1, M, moebius(k) * log(1+x^k/(1-4*x^k))/k))


