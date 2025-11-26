\\ a(n) = Sum_{k=0..floor(n/3)} (5*k+1) * binomial(2*n-k+1,n-3*k)/(2*n-k+1).
a(n) = sum(k=0, n\3, (5*k+1)*binomial(2*n-k+1, n-3*k)/(2*n-k+1));
for(n=0, 28, print1(a(n), ", "));

