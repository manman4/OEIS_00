M=25;

my(A=1, n=M); for(i=1, n, A=( 1 + 4*x*A^4 )^(1/2) +x*O(x^n) ); Vec(A)

\\ G.f. A(x) satisfies A(x) = 1/A(-x*A(x)^6).
my(A=1, n=M); for(i=1, n, A=( 1 + 4*x*A^4 )^(1/2) +x*O(x^n) ); Vec(1/subst(A, x, -x*A^6))
