\\ a(n) = (n+2) * a(n-1) - (n-1) * a(n-2) for n > 3.
a(n) = my(v=[1,5,21]); if(n<4, v[n], (n+2)*a(n-1) - (n-1)*a(n-2) );
for(n=1, 20, print1(a(n),", "))

\\ for n > 2はダメ
a(n) = my(v=[1,5,21]); if(n<3, v[n], (n+2)*a(n-1) - (n-1)*a(n-2) );
for(n=1, 20, print1(a(n),", "))