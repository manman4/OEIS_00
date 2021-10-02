M=14;

\\ b(n) = Sum_{k=1..n} ((-1)^(k-1)+1)/(2^k*k!) * ( Sum_{i=0..k} (-1)^i*(k-2*i)^n *binomial(k,i) ) * ( Sum_{j=1..k} j! * 2^(k-j-1) * (-1)^((k+1)/2+j) * stirling2(k,j) )
a(n) = sum(k=1, n, ((-1)^(k-1)+1)/(2^k*k!) * ( sum(i=0, k, (-1)^i*(k-2*i)^n *binomial(k,i)) ) * ( sum(j=1, k, j! * 2^(k-j-1) * (-1)^((k+1)/2+j) * stirling(k,j,2)) ));
for(n=0, M, print1(a(2*n+1), ", "));

a(2*100+1)