T(n, k) = binomial(n-1, n\2-k)-binomial(n-1, n\2-k-2);
for(n=0, 14, for(k=0, n\2, print1(T(n, k), ", "))); 

\\ 同じ結果
S(n, k) = if( k<0 || 2*k>n, 0, polcoeff((1 - x) * (1 + x)^n, n\2 - k));
for(n=0, 25, for(k=0, n\2, print1(T(n, k)-S(n, k), ", ")));