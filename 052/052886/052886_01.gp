\\ E.g.f. satisfies A(x) = (exp(x) - 1) / (1 - A(x)).
seq(n) = my(A=x); for(i=1, n, A=(exp(x + x*O(x^n)) - 1) / (1 - A) ); Vec(serlaplace(A));
concat(0, seq(20))   