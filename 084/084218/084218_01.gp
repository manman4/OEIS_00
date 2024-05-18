a(n) = sumdiv(n, d, moebius(n/d) * (n/d)^2 * sigma(d, 4));
for(n=1, 100, print1(a(n), ", "))
