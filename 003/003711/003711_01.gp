v(n)={x='x+O('x^(n+10)); cos(tanh(x))};
M=100;
v=v(2*M);
for(n=0, M, i=(2*n)!*polcoef(v, 2*n); if((i<0)+#digits(i)>1000, break); write("/Users/xxx/Desktop/b003711_1.txt", n, " ", i))