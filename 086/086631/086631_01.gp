a(n) = sum(k=0, n\2, binomial(n+2*k+1,4*k+1) * binomial(3*k,k) / (2*k+1) );
for(n=0, 20, print1(a(n),", "))     

seq(n) = my(A=1); for(i=1, n, A= 1/(1 - x +x*O(x^n))^2 +x^2*A^3  ); Vec(A); 
seq(20) 

