a(n) = (1/(n+1)!) * sum(k=0, n, (n+k)! * stirling(n, k, 2));
for(n=0, 30, print1(a(n),", "))     