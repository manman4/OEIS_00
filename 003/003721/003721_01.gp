v(n)={x='x+O('x^(n+10)); tan(tanh(x))};
M=100;
v=v(2*M+1);
for(n=0, M, i=(2*n+1)!*polcoef(v, 2*n+1); if((i<0)+#digits(i)>1000, break); write("/Users/xxx/Desktop/b003721_1.txt", n, " ", i))