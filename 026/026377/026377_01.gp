my_beselli(n, x) = besseli(n, x)*(x/2)^n/n!;

\\ E.g.f.: exp(3x) * BesselI(2, 2x).
my(N=30, x='x+O('x^N)); Vec(serlaplace(exp(3*x) * my_beselli(2, 2*x)))
my(N=30, x='x+O('x^N)); serlaplace(exp(3*x) * my_beselli(2, 2*x))
