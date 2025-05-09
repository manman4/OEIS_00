\\ G.f. of column k: x^k * exp( Sum_{j>=1} f((k+1)*j)/f(j) * x^j/j ), where f(j) = 4^j - 1.
T(n, k) = if(n==0 && k==0, 1,my(x='x+O('x^(n+2)), f(j) = 4^j - 1); polcoef(  x^k * exp( sum(j=1, n, f((k+1)*j)/f(j) * x^j/j) ), n));
for(n=0, 10, for(k=0, n, print1(T(n, k),", ")));