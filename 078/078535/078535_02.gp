\\ G.f.: ( (1/x) * Series_Reversion(x/(1+36*x)^(7/6)) )^(1/7).
my(N=30, x='x+O('x^N)); Vec( ( serreverse(x/(1+36*x)^(7/6))/x )^(1/7) )

