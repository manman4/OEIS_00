M=30;

a_vector(n) = my(v=vector(n+1)); v[1]=1; for(i=1, n, v[i+1]=sum(j=1, i, sum(k=0, j, k!*abs(stirling(j, k, 1)))*binomial(i-1, j-1)*v[i-j+1])); v;
a_vector(M)