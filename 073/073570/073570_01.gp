M=50;

a(n) = sumdiv(n, d, binomial(d+3, 4)); 
for(n=1, M, print1(a(n),", "));