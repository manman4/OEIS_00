\\ E.g.f.: Series_Reversion( x / (x + exp(x)) ). 
my(N=30, x='x+O('x^N)); concat(0, Vec(serlaplace(serreverse( x / (x + exp(x)) ))))

