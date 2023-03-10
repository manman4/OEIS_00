M=999;

a(n) = fromdigits(Vec(Pol(digits(11))^n)%10);
for(n=0, M, write("/Users/xxx/Desktop/b059734_01.txt", n, " ", a(n)))