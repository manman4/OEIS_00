M=30;

a_vector(n) = my(v=vector(n)); v[1]=1; for(i=1, n-1, v[i+1]=sum(j=1, i, stirling(i, j, 2)*v[j])); v;
a_vector(M)