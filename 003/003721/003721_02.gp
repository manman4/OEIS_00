M=14;

\\ a(n) = Sum_{i=0..(n-1)} ( ( Sum_{j=1..2*i+1} j!*2^(2*i+1-j-1)*(-1)^(i+j+1)*Stirling2(2*i+1,j) ) * Sum_{k=2*i+1..2*n-1} binomial(k-1,2*i)*k!*(-1)^(1+k)*2^(2*n-k-1)*Stirling2(2*n-1,k) )/(2*i+1)!. 
a(n) = sum(i=0, n-1, ( ( sum(j=1, 2*i+1, j!*2^(2*i+1-j-1)*(-1)^(i+j+1)*stirling(2*i+1, j, 2)) ) * sum(k=2*i+1, 2*n-1, binomial(k-1,2*i)*k!*(-1)^(1+k)*2^(2*n-k-1)*stirling(2*n-1, k, 2)) )/(2*i+1)!);
for(n=0, M, print1(2*a(n+1), ", "));

2*a(100+1)