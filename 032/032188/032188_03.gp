M=300;

\\ a(n) = Sum_{k>=0} (1/2)^(n+k) * |Stirling1(n-1+k,k)|. 
a(n) = sum(k=0, M, (1/2.)^(n+k) * abs(stirling(n-1+k, k, 1)) );
for(n=1, 15, print1(a(n),", "));


a(n) = round( sum(k=0, M, (1/2)^(n+k) * abs(stirling(n-1+k, k, 1)) ) );
for(n=1, 15, print1(a(n),", "));

