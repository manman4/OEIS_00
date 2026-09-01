T(n, k) = if(n==1 && k==0, 1, polcoef(polcoef(sum(j=0, n, j!*(((1-y)*(2*x^2-x^3)-x)/((1-y)*x^2-1+O(x^(n+1))))^j), n), k)/2);
cnt = 1;
for(n=1, 150, for(k=0, n\2, write("b010030_1.txt", cnt, " ", T(n,n\2-k)); cnt++));

