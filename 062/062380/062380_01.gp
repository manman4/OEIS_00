\\ a(n) = Sum_{k=1..n} tau( (n/gcd(k,n))^2 ).
a(n) =sum(k=1, n, numdiv((n/gcd(k,n))^2));

for(n=1, 100, print1(a(n), ", "))
