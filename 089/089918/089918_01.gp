\\ a(0) = 1; a(n) = n! * [x^n] (1/n) * exp(n*x) * Sum_{k=0..n-1} a(k)*x^k/k!. 
a(n) = if(n==0, 1, n!*polcoef(exp(n*x+O(x^(n+5))) * sum(k=0, n-1, a(k)*x^k/k!), n)/n);                                                     
for(n=0, 19, print1(a(n),", ")); 