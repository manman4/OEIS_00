\\ G.f.: ( Sum_{k in Z} x^(2*k) / (1 - x^(5*k+2)) ) / ( Sum_{k in Z} x^k / (1 - x^(5*k+1)) ).
my(N=60, x='x+O('x^N)); Vec(sum(k=-N, N, x^(2*k)/(1-x^(5*k+2)))/sum(k=-N, N, x^k/(1-x^(5*k+1))))


