M=46;

a(n) = sum(k=1, n^2, sumdiv(k, d, kronecker(-4, k/d)));
for(n=0, M, print1(a(n), ", "));