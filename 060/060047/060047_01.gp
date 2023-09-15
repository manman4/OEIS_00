\\ G.f. for k-th diagonal: (-1)^k * (1/k) * ( Sum_{j>=k} (-1)^j * j * binomial(j+k-1,2*k-1) * q^(j^2) ) / ( 1 + 2 * Sum_{j>=1} (-q)^(j^2) ). 
T(n, k) = my(q='q+O('q^(n+10))); polcoef( (-1)^k * (1/k) * ( sum(j=k, n, (-1)^j * j * binomial(j+k-1,2*k-1) * q^(j^2)) ) / ( 1 + 2 * sum(j=1, n, (-q)^(j^2)) ) , n);
for(n=0, 24, for(k=1, sqrtint(n), print1(T(n, sqrtint(n) + 1 - k), ", ")))