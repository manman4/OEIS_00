my(N=100, x='x+O('x^N)); Vec(serlaplace(cos(x)))  

my(N=100, x='x+O('x^N)); serlaplace(cos(x))



a(n) = (I^n + (-I)^n)/2;
for(n=0, 100, print1(a(n),", "))

my(N=100, x='x+O('x^N)); Vec(1/(1 + x^2))  
