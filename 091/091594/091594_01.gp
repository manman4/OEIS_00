T(n, k) = my(x='x+O('x^(n+20))); polcoef(1/(1-x-x^2)*(x*(1-x^2)/(1-x-x^2))^k, n);
for(n=0, 11, for(k=0, n, print1(T(n, k), ", ")));
