M=25;

a(n) = my(v=[1,0,0,0,2]); if(n<5, v[n+1], (2*(n-1)*a(n-1) - (n-2)*a(n-2) + 2*(2*n-4)*a(n-4) - 2*(2*n-7)*a(n-5))/n);  
for(n=0, M, print1(a(n), ", "));  

\\ 上記を変形
a(n) = my(v=[1,1,3,5]); if(n<4, v[n+1], ((3*n-2)*a(n-1) + n*a(n-2) - (7*n-12)*a(n-3) + 2*(2*n-5)*a(n-4))/n);  
for(n=0, M, print1(a(n), ", "));  

\\ こちらの方が簡単
a(n) = my(v=[1,1,3]); if(n<3, v[n+1], ((2*n-1)*a(n-1) + (3*n-3)*a(n-2) - (4*n-6)*a(n-3))/n);  
for(n=0, M, print1(a(n), ", "));  