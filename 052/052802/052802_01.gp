a(n) = (1/(n+1)!) * sum(k=0, n, (n+k)! * abs(stirling(n, k, 1)));
for(n=0, 30, print1(a(n),", "))     