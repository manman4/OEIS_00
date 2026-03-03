\\ a(0) = 1; a(n) = 2 - n + Sum_{k=0..n-1} a(k) * a(n-1-k).
a(n) = if(n==0, 1, 2 - n + sum(k=0, n-1, a(k)*a(n-1-k)));
for(n=0, 15, print1(a(n),", "));                    