M=470;
a(n) = n!*polcoef((1+x+x*O(x^n))^(1-x), n);

for(n=0, M, i=a(n); if((i<0)+#digits(i)>1000, break); write("/Users/xxx/Desktop/b007120_1.txt", n, " ", a(n)))

