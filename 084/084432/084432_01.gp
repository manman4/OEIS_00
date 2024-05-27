 for(n=1, 500, l=ceil(log(n)/log(2)); t=polcoeff(sum(k=0, l, 1/(1-x^2^k)^2) + O(x^(n+1)), n); print1(t", "))

a(n) = my(A=x); for(i=1, n, A = 1/(1-x +x*O(x^n))^2-1 + subst(A, x, x^2) ); Vec(A);
a(500)      