\\ a(n) = n * ( n*a(n-1) - (n-1)*(n-2)/2 * a(n-2) ) for n > 1. 
a(n) = if(n<2, 1, n * ( n*a(n-1) - (n-1)*(n-2)/2 * a(n-2) ));
for(n=0, 20, print1(a(n),", "))