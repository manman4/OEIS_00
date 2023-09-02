a(n, k=3) = sumdiv(n+1, d, (d%2)*eulerphi(d)*moebius(d/gcd(d, k))/eulerphi(d/gcd(d, k))*2^((n+1)/d))/(2*(n+1)); 
for(n=0, 34, print1(a(n),", ")) 
