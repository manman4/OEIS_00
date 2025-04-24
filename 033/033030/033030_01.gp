\\ E.g.f.: B(x)^3, where B(x) is the e.g.f. of A381484.
my(N=20, x='x+O('x^N)); Vec(serlaplace( ( exp(-x/3)/(1-3*x)^(1/9) )^3 ))