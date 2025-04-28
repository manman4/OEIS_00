M=21;

\\ a(n) = (n^2 * a(n-1) + 2)/(n-1) for n > 2. 
a(n) = my(v=[1,0]); if(n<3, v[n], (n^2*a(n-1) + 2)/(n-1));
for(n=1, M, print1(a(n),", "))

\\ for n > 1はダメ
a(n) = my(v=[1,0]); if(n<2, v[n], (n^2*a(n-1) + 2)/(n-1));
for(n=1, M, print1(a(n),", "))


