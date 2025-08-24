my_beselli(n, x) = besseli(n, x)*(x/2)^n/n!;

\\ E.g.f.: BesselI(4,2*x) * exp(2*x).
my(N=30, x='x+O('x^N)); Vec(serlaplace(exp(2*x) * my_beselli(4, 2*x)))
my(N=30, x='x+O('x^N)); serlaplace(exp(2*x) * my_beselli(4, 2*x))