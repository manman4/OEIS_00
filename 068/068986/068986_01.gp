\\ Also numbers k such that k divides A060640(k).

a060640(n) = n*sumdiv(n, d, sigma(d)/d); 
for(n=1, 100000, if(a060640(n)%n==0, print1(n, ", ")));