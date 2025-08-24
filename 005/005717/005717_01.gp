my_beselli(n, x) = besseli(n, x)*(x/2)^n/n!;

\\ E.g.f.: exp(x) * I_1(2x), where I_1 is the Bessel function. 
my(N=30, x='x+O('x^N)); Vec(serlaplace(exp(x) * my_beselli(1, 2*x)))
my(N=30, x='x+O('x^N)); serlaplace(exp(x) * my_beselli(1, 2*x))