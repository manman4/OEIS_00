\\ A(0,k) = 1 and A(n,k) = -(k*(k+1) * A(n-1,k+2) + 4*k^2 * A(n-1,k)) for n > 0. a(n) = A(n,1).
a(n, k=1) = if(n==0, 1, -(k*(k+1)*a(n-1, k+2) + 4*k^2*a(n-1, k)));
for(n=0, 18, print1(a(n),", "));

