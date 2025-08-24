my_besseli(n, x) = besseli(n, x)*(x/2)^n/n!;

\\ E.g.f.: BesselI(11,2*x) * exp(2*x).
my(N=30, x='x+O('x^N)); Vec(serlaplace(exp(2*x) * my_besseli(11, 2*x)))
my(N=30, x='x+O('x^N)); serlaplace(exp(2*x) * my_besseli(11, 2*x))