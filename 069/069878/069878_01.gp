default(parisize, 12000000000)

M=6;
a(n) = my(A); if( n<0, 0, A = x * O(x^n); polcoeff( eta(x^2 + A) / eta(x + A), n));

for(n=0, M, write("/Users/xxx/Desktop/b069878_1.txt", n, " ", a(10^n)))