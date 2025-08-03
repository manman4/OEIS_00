M=25;

\\ a(n) = [x^n] 1/((1-6*x) * (1-x)^(5*n+1)).
a(n) = polcoef( 1/( (1-6*x) * (1-x)^(5*n+1) +x*O(x^n) ), n);
for(n=0, M, print1(a(n)", "));

