\\ G.f.: Sum_{i>=1, j>=0} x^(i*5^j). 
my(N=100, x='x+O('x^N)); Vec( sum(i=1, N, sum(j=0, 5, x^(i*5^j) )) )
