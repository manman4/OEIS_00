a001353(n) = polchebyshev(n-1, 2, 2);
for(n=0, 49, print1(a001353(n), ", "));

\\ a(n) = A001353(n) * A001353(n+1) / 4.
a(n) = a001353(n) * a001353(n+1) / 4;
for(n=0, 29, print1(a(n), ", "));

