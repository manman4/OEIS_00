\\ E.g.f.: -4 - 3*x/2 + (-19*x/2 + 4*exp(x))/(1-x)^2.
my(N=30, x='x+O('x^N)); Vec(serlaplace( -4 - 3*x/2 + (-19*x/2 + 4*exp(x))/(1-x)^2 ))
