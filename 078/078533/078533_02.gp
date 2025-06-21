\\ G.f.: ( (1/x) * Series_Reversion(x/(1+16*x)^(5/4)) )^(1/5).
my(N=30, x='x+O('x^N)); Vec( ( serreverse(x/(1+16*x)^(5/4))/x )^(1/5) )

