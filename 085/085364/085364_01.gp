a(n) = if(n==0, 1, (6/n) * sum(k=0, n-1, (n+k) * a(k)));
for(n=0, 20, print1(a(n),", "))   
