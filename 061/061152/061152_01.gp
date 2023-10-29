b(k) = sumdiv(k, d, (-1)^(k/d+1)*d*prime(d));    
a(n) = if(n==0, 1, (1/n)*sum(k=1, n, a(n-k)*b(k)));
for(n=0, 25, print1(a(n),", "));