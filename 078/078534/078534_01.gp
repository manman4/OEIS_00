M=25;

\\ G.f. A(x) satisfies A(x) = (1 + 25*x*A(x)^6)^(1/5)
my(A=1, n=M); for(i=1, n, A=(1 + 25*x*A^6)^(1/5) + x*O(x^n)); Vec(A)

\\ G.f. A(x) satisfies A(x) = 1/A(-x*A(x)^7).
my(A=1, n=M); for(i=1, n, A=(1 + 25*x*A^6)^(1/5) + x*O(x^n)); Vec(A - 1/subst(A, x, -x*A^7))

