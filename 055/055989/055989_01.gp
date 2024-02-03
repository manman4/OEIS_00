\\ Number of compositions of 4*n-3 into parts 1 and 4.
a(n) = polcoef((1/(1 - x - x^4) + O(x^1000)), 4*n-3); 
for(n=1, 25, print1(a(n),", "))  