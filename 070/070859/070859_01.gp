M=26;

\\ Expansion of (1+x*C^4)*C, where C = (1-(1-4*x)^(1/2))/(2*x) is g.f. for Catalan numbers, A000108.
my(N=M, x='x+O('x^N), g=sum(k=0, N, binomial(2*k, k)/(k+1)*x^k)); Vec( (1+x*g^4)*g )

\\ a(n) = 3 * (7*n^3+13*n^2+12*n+8) * (2*n)!/(n! * (n+4)!).
a(n) = 3 * (7*n^3 + 13*n^2 + 12*n + 8) * (2*n)!/(n! * (n + 4)!);
for(n=0, M, print1(a(n),", "));

\\ a(n) = binomial(2*n,n) + 5*binomial(2*n,n-2)/(n+3) - binomial(2*n,n-4).
b(n) = binomial(2*n, n) + 5*binomial(2*n, n-2)/(n+3) - binomial(2*n, n-4);
for(n=0, 100, print1(a(n)-b(n),", "));