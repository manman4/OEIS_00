\\ a(n) = [x^n] 1/((1-x)^(n+1) * (1-2*x)^n).
a(n) = my(x='x+O('x^(n+1))); polcoef( 1/((1-x)^(n+1) * (1-2*x)^n), n);
for(n=0, 20, print1(a(n), ", "));



