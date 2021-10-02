M=14;

\\ a(n) = 2 * Sum_{m=0..n} ( Sum_{j=2*m..2*n} binomial(j-1,2*m-1) * j! * 2^(2*n-j-1) * (-1)^(n+j) * Stirling2(2*n,j) )/(2*m)!, n>0, a(0)=1
a(n) = if(n==0, 1, 2*sum(m=0, n, ( sum(j=2*m, 2*n, binomial(j-1, 2*m-1)*j!*2^(2*n-j-1)*(-1)^(n+j)*stirling(2*n, j, 2)) )/(2*m)!));
for(n=0, M, print1(a(n), ", "));