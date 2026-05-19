\\ G.f.: x * (1-x)^6 / (1-4*x+3*x^2-x^3)^3. 
my(N=30, x='x+O('x^N)); x * (1-x)^6 / (1-4*x+3*x^2-x^3)^3
my(N=30, x='x+O('x^N)); Vec(x * (1-x)^6 / (1-4*x+3*x^2-x^3)^3)


\\ A052544 Expansion of (1-x)^2/(1 - 4*x + 3*x^2 - x^3).
my(N=30, x='x+O('x^N)); Vec((1-x)^2/(1 - 4*x + 3*x^2 - x^3))

\\ G.f.: x * B(x)^3, where B(x) is the g.f. of A052544.
my(N=30, x='x+O('x^N), B=(1-x)^2/(1 - 4*x + 3*x^2 - x^3)); x * B^3
my(N=30, x='x+O('x^N), B=x * (1-x)^2/(1 - 4*x + 3*x^2 - x^3)); Vec(x * B^3)