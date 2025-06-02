M=17;

\\ a(n) = (-1)^(n+1)/6 * Li_{-n}(6/5), where Li_{n}(x) is the polylogarithm function.
a(n) = (-1)^(n+1)/6 * polylog(-n, 6/5);
for(n= 0, M, print1(a(n), ", "));