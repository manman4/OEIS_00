\\ a(n) = [x^n] (1+4*x+5*x^2)^n.
a(n) = polcoef((1+4*x+5*x^2)^n, n);
for(n=0, 23, print1(a(n),", "));

