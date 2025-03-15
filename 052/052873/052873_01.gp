\\ E.g.f.: exp( Series_Reversion( x*exp(-x)/(1+x) ) ). 
my(N=20, x='x+O('x^N)); Vec(serlaplace(exp( serreverse(x*exp(-x)/(1+x)) )) )

