\\ a(n) = n! * Sum_{j=0..n} Sum_{k=0..j} binomial(j,n-j-k) * |Stirling1(j,k)|/j!. 
a(n) = n!*sum(j=0, n, sum(k=0, j, binomial(j, n-j-k)*abs(stirling(j, k, 1))/j! ));
for(n=0, 25, print1(a(n),", "))  
