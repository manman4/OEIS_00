M=20;

\\ a(0) = 1; a(n) = Sum_{k=0..floor((n-1)/2)} a(2*k) * a(n-1-2*k).
a(n) = if(n==0, 1, sum(k=0, (n-1)\2, a(2*k) * a(n-1-2*k)));
for(n=0, M, print1(a(n),", "));