default(realprecision, 120);

M=1000;

a(n) = floor(abs(1/sin(n)));
for(n=1, M, write("/Users/xxx/Desktop/b086867_1.txt", n, " ", a(n)))