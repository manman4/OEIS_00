M=20;

\\ a(n) = -(n-3)*a(n-1) + 3*(n-1)*a(n-2) + 2*(n-1)*(n-2)*a(n-3) for n > 2.
a(n) = my(v=[1,2,5]); if(n<3, v[n+1], -(n-3)*a(n-1) + 3*(n-1)*a(n-2) + 2*(n-1)*(n-2)*a(n-3));
for(n=0, M, print1(a(n),", "))                 

