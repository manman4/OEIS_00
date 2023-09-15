\\ G.f. for k-th diagonal: (-1)^k * (1/(2*k+1)) * ( Sum_{j>=k} (-1)^j * (2*j+1) * binomial(j+k,2*k) * q^(j*(j+1)/2) ) / ( Sum_{j>=0} (-1)^j * (2*j+1) * q^(j*(j+1)/2) ).
T(n, k) = my(q='q+O('q^(n+10))); polcoef( (-1)^k * (1/(2*k+1)) * ( sum(j=k, n, (-1)^j * (2*j+1) * binomial(j+k,2*k) * q^(j*(j+1)/2)) ) / ( sum(j=0, n, (-1)^j * (2*j+1) * q^(j*(j+1)/2)) ) , n);
a(n) = s=0; while((s+1)*(s+2)/2<=n, s++); s;
for(n=1, 19, for(k=1, a(n), print1(T(n, a(n) + 1 - k), ", ")))