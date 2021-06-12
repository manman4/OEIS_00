M=44;

a(n) = sum(k=1, n, sumdiv(k, d, moebius(k/d)*binomial(d+1, 2))); 
for(n=1, M, print1(a(n), ", "));