M=20;

a(n) = my(v=[1,-4]); if(n<2, v[n+1], (-2*(3*n-1)*a(n-1) + 7*(n-2)*a(n-2))/n);  
for(n=0, M, print1(a(n), ", "));