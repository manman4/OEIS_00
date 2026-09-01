M=450;

\\ G.f.: x^2*Sum_{k>=0} k*k!*(x^2-x+k-1)*(-x*(x-1)/(x+1))^k/((x^2-1)^2*(x-1)^2).
v(n) = my(x='x+O('x^(n+8))); x^2*sum(k=0, n, k*k!*(x^2-x+k-1)*(-x*(x-1)/(x+1))^k/((x^2-1)^2*(x-1)^2));
v = v(M);
cnt = 4;
for(n=4, M, write("b000544_1.txt", cnt, " ", polcoef(v, n)); cnt++);