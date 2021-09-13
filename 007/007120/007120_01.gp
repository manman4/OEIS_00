M=454;
a(n) = n!*polcoef((1+x+x*O(x^n))^(1-x), n);

for(n=0, M, write("/Users/xxx/Desktop/b007120_1.txt", n, " ", a(n)))

