\\ a(0) = 0; a(n) = -n*a(n-1) + binomial(n+1,2).
a(n) = if(n==0, 0, -n*a(n-1) + binomial(n+1,2));
for(n=0, 20, print1(a(n), ", "));