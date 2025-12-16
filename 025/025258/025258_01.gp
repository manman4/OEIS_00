M=50;

\\ G.f. A(x) satisfies A(x) = x * (1-x + x^2/(1-A(x))).
my(A=x, n=M); for(i=1, n, A = x * (1-x + x^2/(1-A)) +x*O(x^n) ); A
my(A=x, n=M); for(i=1, n, A = x * (1-x + x^2/(1-A)) +x*O(x^n) ); Vec(A)
