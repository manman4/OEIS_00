\\ A(0,k) = 1 and A(n,k) = -(k*(k+1) * A(n-1,k+2) + k^2 * A(n-1,k)) for n > 0. a(0) = 0 and a(n) = A(n-1,2).
A(n,k) = if(n==0, 1, -(k*(k+1)*A(n-1,k+2) + k^2*A(n-1,k)));
a(n) = if(n==0, 0, A(n-1,2));
for(n=0, 18, print1(a(n),", "));

