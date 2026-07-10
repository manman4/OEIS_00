\\ a(n) = Sum_{d|n} d^3 * J_3(d), where J_k is the k-th Jordan totient function.
J(n, k) = sumdiv(n, d, d^k*moebius(n/d));
a(n) = sumdiv(n, d, d^3 * J(d, 3));
for(n=1, 30, print1(a(n), ", "));

