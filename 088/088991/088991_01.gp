\\ E.g.f.: B(x)^4, where B(x) is the e.g.f. of A381504.
my(N=20, x='x+O('x^N)); Vec(serlaplace( ( exp(-x/4)/(1-4*x)^(1/16) )^4 ))