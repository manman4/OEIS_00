M=10;

\\ G.f.: Product_{k>=1} 1/(1 - k*x)^((1/6) * (5/6)^k).
a(n) = my(x='x+O('x^(n+1))); polcoef(prod(k=1, 1000, 1/(1 - k*x)^((1/6) * (5/6)^k)), n) + 0.;
for(n=0, M, print1(round(a(n)),", "));
