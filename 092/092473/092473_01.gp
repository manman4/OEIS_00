M=200;

a(n)=sum(i=0, n, sum(j=0, i, sum(k=0, j, if(i+j+k-n, 0, (2*n)!/((i+j)!*(j+k)!*(k+i)!)))));
for(n=0, M, i=a(n); if((i<0)+#digits(i)>1000, break); write("/Users/xxx/Desktop/b092473_1.txt", n, " ", i))