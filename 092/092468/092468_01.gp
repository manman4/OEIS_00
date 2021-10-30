M=200;

a(n)=sum(i=0, n, sum(j=0, i, sum(k=0, j, if(i+j+k-n, 0, (n+2*k)!/(i!*j!*(3*k)!)))));
for(n=0, M, i=a(n); if((i<0)+#digits(i)>1000, break); write("/Users/xxx/Desktop/b092468_1.txt", n, " ", i))