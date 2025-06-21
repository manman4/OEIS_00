M=25;

\\ G.f. A(x) satisfies A(x) = (1 + 16*x*A(x)^5)^(1/4)
my(A=1, n=M); for(i=1, n, A=(1 + 16*x*A^5)^(1/4) + x*O(x^n)); Vec(A)

\\ G.f. A(x) satisfies A(x) = 1/A(-x*A(x)^6).
my(A=1, n=M); for(i=1, n, A=(1 + 16*x*A^5)^(1/4) + x*O(x^n)); Vec(A - 1/subst(A, x, -x*A^6))

