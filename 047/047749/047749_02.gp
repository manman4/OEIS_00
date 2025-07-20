M=40;

apr(n, p, r) = r*binomial(n*p+r, n)/(n*p+r);
a(n) = apr(n\2, 3, n%2+1); 
for(n=0, M, print1(a(n),", "));