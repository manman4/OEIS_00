\\ E.g.f.: 1/(1 - 2 * sin(x)).
my(N=20, x='x+O('x^N)); Vec(serlaplace(1/(1-2*sin(x))))
