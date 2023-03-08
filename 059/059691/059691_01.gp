M=9999;

T(n, k) = fromdigits(Vec(Pol(digits(n))*Pol(digits(k)))%10);
for(n=0, M, write("/Users/xxx/Desktop/b059691_01.txt", n, " ", T(12, n)))