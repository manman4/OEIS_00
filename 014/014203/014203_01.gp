\\ G.f.: theta_3(x)^3
my(N=50, x='x+O('x^N)); Vec( 1 + 2 * sum(k=1, N, x^k^2) )

\\ G.f.: (x/(1-x)) * d/dx(theta_3(x)^3)
my(N=50, x='x+O('x^N)); concat(0, Vec(x/(1-x)*deriv( ( 1 + 2 * sum(k=1, N, x^k^2) )^3 )))