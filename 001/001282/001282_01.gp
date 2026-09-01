M=450;

\\ G.f.: (1/(1-x)^9) * Sum_{k>=0} k! * ((1+x)^2*binomial(k+1,1)^2 - 8*(1+x)*binomial(k+2,2)^2 + 24*binomial(k+3,3)^2) * (x*(1-x)/(1+x))^(k+6). 
v(n) = my(x='x+O('x^(n+8))); sum(k=0, n, k!*((1+x)^2*binomial(k+1,1)^2 - 8*(1+x)*binomial(k+2,2)^2 + 24*binomial(k+3,3)^2)*(x*(1-x)/(1+x))^(k+6))/(1-x)^9;
v = v(M);
cnt = 6;
for(n=6, M, write("b001282_1.txt", cnt, " ", polcoef(v, n)); cnt++);