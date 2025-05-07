pell(n) = ([2, 1; 1, 0]^n)[2, 1];
\\ G.f.: x^4 * exp( Sum_{k>=1} Pell(5*k)/Pell(k) * x^k/k ).
my(N=30, x='x+O('x^N)); Vec( x^4 * exp( sum(k=1,N, pell(5*k)/pell(k) * x^k/k )) )


