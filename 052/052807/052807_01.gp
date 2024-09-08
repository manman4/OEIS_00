\\ E.g.f.: Series_Reversion( 1 - exp(-x*exp(-x)) ). 
my(N=20, x='x+O('x^N)); concat(0, Vec(serlaplace( serreverse( 1 - exp(-x*exp(-x)) ) )))