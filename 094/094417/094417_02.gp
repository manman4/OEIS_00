M=17;

\\ a(n) = (-1)^(n+1)/5 * Li_{-n}(5/4), where Li_{n}(x) is the polylogarithm function.
a(n) = (-1)^(n+1)/5 * polylog(-n, 5/4);
for(n= 0, M, print1(a(n), ", "));