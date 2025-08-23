\\ E.g.f.: x*exp(3x)*I_1(2x), where I_1 is Bessel function. 
my(N=30, x='x+O('x^N)); Vec(serlaplace(x*exp(3*x)*besseli(1, 2*x)))
my(N=30, x='x+O('x^N)); serlaplace(x*exp(3*x)*besseli(1, 2*x))
