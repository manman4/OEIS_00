b(n) = if(n<0, 0, n!*polcoeff(1/(2-exp(exp(x+x*O(x^n))-1)), n));
a(n) = sum(k=0, n, stirling(n,k,2) * b(k));
for(n=0, 19, print1(a(n),", "))   