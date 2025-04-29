\\ Diagonal of the rational function 1 / ((1-x)*(1-y) - x^2*y^3).
a(n) = my(x='x+O('x^(n+1)), y='y+O('y^(n+1)) ); polcoef(polcoef(1 / ((1-x)*(1-y) - x^2*y^3), n), n);
for(n=0, 35, print1(a(n),", "))         