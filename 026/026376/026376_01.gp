my_besseli(n, x) = besseli(n, x)*(x/2)^n/n!;

\\ E.g.f.: exp(3x)*I_1(2x), where I_1 is Bessel function.
my(N=30, x='x+O('x^N)); Vec(serlaplace(exp(3*x) * my_besseli(1, 2*x)))
my(N=30, x='x+O('x^N)); serlaplace(exp(3*x) * my_besseli(1, 2*x))
