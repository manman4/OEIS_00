K=4;
M=17;

a(n) = my(A=1); A = sum(k=0, n, binomial(K*k+1, k)/(K*k+1)*x^k) + x*O(x^n); Vec(1/subst(A, x, -x*A^(2*K-1)));
print1(a(M));
