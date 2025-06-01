M=17;

\\ a(n) = (-1)^(n+1)/7 * Li_{-n}(7/6), where Li_{n}(x) is the polylogarithm function.
a(n) = (-1)^(n+1)/7 * polylog(-n, 7/6);
for(n= 0, M, print1(a(n), ", "));