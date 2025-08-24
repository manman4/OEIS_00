my_besesli(n, x) = besseli(n, x)*(x/2)^n/n!;

\\ E.g.f.: BesselI(6,2*x) * exp(2*x).
my(N=30, x='x+O('x^N)); Vec(serlaplace(exp(2*x) * my_besseli(6, 2*x)))
my(N=30, x='x+O('x^N)); serlaplace(exp(2*x) * my_besseli(6, 2*x))