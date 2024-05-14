default(parisize, 10000000000) 

a(n) = my(v=vector(2*n^3, i, 0)); for(i=1, n, for(j=i, n, v[i^3+j^3]+=1)); sum(i=1, #v, v[i]>1); 
print(a(436))