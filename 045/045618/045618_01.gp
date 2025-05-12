\\ a(n) = Sum_{k=0..n} Stirling2(k+2,2) * Stirling2(n-k+2,2).
a(n) = sum(k=0, n, stirling(k+2,2,2) * stirling(n-k+2,2,2));
for(n=0, 20, print1(a(n),", "))                   

