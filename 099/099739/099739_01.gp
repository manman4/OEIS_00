M=20;

a(n) = sum(k=1, n, moebius(k)*k!); 
for(n=1, M, print1(a(n), ", "));