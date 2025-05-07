pell(n) = ([2, 1; 1, 0]^n)[2, 1];
\\ G.f.: x * exp( Sum_{k>=1} Pell(3*k)/Pell(k) * x^k/k ).
my(N=30, x='x+O('x^N)); concat(0, Vec( x * exp( sum(k=1,N, pell(3*k)/pell(k) * x^k/k )) ))



