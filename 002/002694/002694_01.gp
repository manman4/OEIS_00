\\ E.g.f.: exp(2*x) * BesselI(2, 2*x).
my_besseli(n, x) = besseli(n, x)*(x/2)^n/n!;

my(N=30, x='x+O('x^N)); Vec(serlaplace(exp(2*x) * my_besseli(2, 2*x)))
my(N=30, x='x+O('x^N)); serlaplace(exp(2*x) * my_besseli(2, 2*x))