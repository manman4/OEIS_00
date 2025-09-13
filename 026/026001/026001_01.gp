\\ a(n) = [x^n] (1-x)^n/(1-2*x)^(3*n+1).
a(n) = polcoef( (1-x)^n/(1-2*x + x*O(x^n))^(3*n+1), n);
for(n=0, 20, print1(a(n),", "));


   