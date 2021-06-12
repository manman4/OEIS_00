M=36;

a(n) = sum(k=1, n, sumdiv(k, d, moebius(k/d)*binomial(d+3, 4))); 
for(n=1, M, print1(a(n), ", "));