a(n) = local(A=x); for(i=1, n, A=x*(1-x) - sum(k=2, n, subst(A, x, x^k) +x*O(x^n) ) ); polcoeff(A, n);  

for(n=1, 30, print1(a(n),", ")) 
