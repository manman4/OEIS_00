T(n, k) = polcoef(polcoef(sum(j=0, n, j!*(((1-y)*(2*x^2-x^3)-x)/((1-y)*x^2-1+O(x^(n+1))))^j), n), k);
for(n=0, 11, print(vector(n\2+1, k, T(n,k-1))));

