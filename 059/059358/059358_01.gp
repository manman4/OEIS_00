M=43;

a(n) = if(n==0, 0, sumdiv(n, d, binomial(d+2, 3))); 
for(n=0, M, print1(a(n),", "));