\\ Diagonal of the rational function 1 / ((1-x^2*y)*(1-x*y^2) - y).
a(n) = my(x='x+O('x^(n+1)), y='y+O('y^(n+1)) ); polcoef(polcoef(1 / ((1-x^2*y)*(1-x*y^2) - y), n), n);
for(n=0, 45, print1(a(n),", "));
  
