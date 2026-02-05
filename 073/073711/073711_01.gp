\\ a(2*n) = a(n) and a(2*n+1) = Sum_{k=0..n} a(k) * a(n-k) with a(0)=1.
a(n) = if(n==0, 1, if(n%2==0, a(n/2), sum(k=0, n\2, a(k)*a(n\2-k))));
for(n=0, 30, print1(a(n),", "))  