\\ E.g.f.: I_3(2*x) * exp(2*x) where I_3 is a modified Bessel function.
my_beselli(n, x) = besseli(n, x)*(x/2)^n/n!;

my(N=30, x='x+O('x^N)); Vec(serlaplace(exp(2*x) * my_beselli(3, 2*x)))
my(N=30, x='x+O('x^N)); serlaplace(exp(2*x) * my_beselli(3, 2*x))