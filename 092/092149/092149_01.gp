a(n) = local(A=x); for(i=1, n, A=x - sum(k=2, n, (1-x^k)*subst(A, x, x^k) +x*O(x^n) ) / (1-x + x*O(x^n))); polcoeff(A, n); 
for(n=1, 30, print1(a(n),", "))
