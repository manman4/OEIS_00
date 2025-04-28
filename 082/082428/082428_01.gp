\\ E.g.f.: -3 - x + (-7*x + 3*exp(x))/(1-x)^2.
my(N=30, x='x+O('x^N)); Vec(serlaplace( -3 - x + (-7*x + 3*exp(x))/(1-x)^2 ))
