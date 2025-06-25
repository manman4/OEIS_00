M=19;

a136630(n, k) = 1/(2^k*k!)*sum(j=0, k, (-1)^(k-j)*(2*j-k)^n*binomial(k, j));
\\ a(n) = Sum_{k=0..n} 2^k * k! * A136630(n,k).
a(n) = sum(k=0, n, 2^k * k! * a136630(n, k));
for(n=0, M, print1(a(n), ", "))

