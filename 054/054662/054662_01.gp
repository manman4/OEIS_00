a(n) = sumdiv(n, d, (gcd(d, 5)==1)*(moebius(d)*5^(n/d)))/(5*n);
for(n=1, 23, print1(a(n),", "))
