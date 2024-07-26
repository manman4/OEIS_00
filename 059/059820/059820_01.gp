M=1000;
D(x, y, n) = sum(k=1, n-1, sigma(k, x)*sigma(n-k, y));
D000(n) = sum(k=1, n-1, sigma(k, 0)*D(0, 0, n-k));
a(n) = if(n==0, 0, (3*D(0, 0, n)+3*D(0, 1, n)+D000(n)+2*sigma(n, 0)+3*sigma(n)+sigma(n, 2))/6);

for(n=0, M, write("/Users/xxx/Desktop/b059820.txt", n, " ", a(n)))

