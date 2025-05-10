\\ Cf. A010048

pell(n) = ([2, 1; 1, 0]^n)[2, 1];

\\ G.f. of column k: x^k * exp( Sum_{j>=1} Pell((k+1)*j)/Pell(j) * x^j/j ).
T(n, k) = my(x='x+O('x^(n+10))); polcoef(x^k * exp( sum(j=1, n+10, pell((k+1)*j)/pell(j) * x^j/j )), n);
for(n=0, 9, for(k=0, n, print1(T(n, k),", ")));

\\ T(n, k) = Pell(n-k-1)*T(n-1, k-1) + Pell(k+1)*T(n-1, k).
S(n,k) = if(n*k==0, n^k, pell(n-k-1)*S(n-1, k-1) + pell(k+1)*S(n-1, k));
for(n=0, 15, for(k=0, n, print1(T(n, k)-S(n, k),", ")));

