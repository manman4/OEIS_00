\\ E.g.f.: x^2/2 * exp(3x) * BesselI(2, 2x). 
my(N=30, x='x+O('x^N)); Vec(serlaplace(x^2/2*exp(3*x)*besseli(2, 2*x)))
my(N=30, x='x+O('x^N)); serlaplace(x^2/2*exp(3*x)*besseli(2, 2*x))
