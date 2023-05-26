a(n) = local(A=1); for(i=1, n, A=exp( sum(k=1, i, subst(A, x, x^k)^3 * x^k / k) + x*O(x^n) )); polcoeff(A, n); 
for(n=0,15,print1(a(n),", "))

