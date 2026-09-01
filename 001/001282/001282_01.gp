M=450;

\\ G.f.: (1/(1-x)^12) * Sum_{k>=0} k! * (-(1+x)^3*binomial(k+1,1)^2 + 12*(1+x)^2*binomial(k+2,2)^2 - 72*(1+x)*binomial(k+3,3)^2 + 192*binomial(k+4,4)^2) * (x*(1-x)/(1+x))^(k+8).
v(n) = my(x='x+O('x^(n+10))); sum(k=0, n, k!*(-(1+x)^3*binomial(k+1,1)^2 + 12*(1+x)^2*binomial(k+2,2)^2 - 72*(1+x)*binomial(k+3,3)^2 + 192*binomial(k+4,4)^2)*(x*(1-x)/(1+x))^(k+8))/(1-x)^12;
v = v(M);
cnt = 8;
for(n=8, M, write("b398330_1.txt", cnt, " ", polcoef(v, n)); cnt++);