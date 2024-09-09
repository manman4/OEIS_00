\\ E.g.f. satisfies A(x) = (log(1 + x)) / (1 - A(x)).
seq(n) = my(A=x); for(i=1, n, A=log(1 + x +x*O(x^n)) / (1 - A) ); Vec(serlaplace(A));
concat(0, seq(20))   