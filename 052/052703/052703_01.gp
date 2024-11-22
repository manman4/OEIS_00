\\ G.f.: RootOf(-_Z+_Z^2+_Z^3+x)-RootOf(-_Z+_Z^2+_Z^3+x)^2-x
my(N=30, x='x+O('x^N)); concat([0, 0, 0], Vec(serreverse(x-x^2-x^3)-serreverse(x-x^2-x^3)^2-x))

\\ G.f.: (x*B(x))^3 where B(x) is the g.f. of A001002.
my(N=30, x='x+O('x^N)); Vec( ( serreverse(x-x^2-x^3)-serreverse(x-x^2-x^3)^2-x )^(1/3) / x )



