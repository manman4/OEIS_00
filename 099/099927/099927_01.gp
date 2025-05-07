\\ G.f. of column k: x^k * exp( Sum_{j>=1} Pell((k+1)*j)/Pell(j) * x^j/j ).
T(n, k) = my(x='x+O('x^(n+10))); polcoef(x^k * exp( sum(j=1, n+10, pell((k+1)*j)/pell(j) * x^j/j )), n);
for(n=0, 9, for(k=0, n, print1(T(n, k),", ")));