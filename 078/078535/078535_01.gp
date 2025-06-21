M=25;

\\ G.f. A(x) satisfies A(x) = ( 1 + 36*x*A(x)^7 )^(1/6).
my(A=1, n=M); for(i=1, n, A=( 1 + 36*x*A^7 )^(1/6) +x*O(x^n) ); Vec(A)

\\ G.f. A(x) satisfies A(x) = 1/A(-x*A(x)^8).
my(A=1, n=M); for(i=1, n, A=( 1 + 36*x*A^7 )^(1/6) +x*O(x^n) ); Vec(A - 1/subst(A, x, -x*A^8))

