\\ E.g.f.: ( -4 + 3*exp(x) + (3 - 2*exp(x))^(3/2) )/3. (see (3.1) in the L. R. Foulds and R. W. Robinson reference).
my(N=25, x='x+O('x^N)); Vec(serlaplace( ( -4 + 3*exp(x) + (3 - 2*exp(x))^(3/2) )/3 ))

my(N=25, x='x+O('x^N)); serlaplace( ( -4 + 3*exp(x) + (3 - 2*exp(x))^(3/2) )/3 )
