a011655(n)=!!(n%3);
a353815(n) = { my(f=factor(n)); prod(i=1, #f~, if(3==f[i, 1], 1, 1==(f[i, 1]%3), 2!=(f[i, 2]%3), (1+f[i, 2])%2)); }; 
a(n) = a011655(n) * a353815(n);
for(n=1, 100, print1(a(n), ", "));

A070563(n) = !!(ramanujantau(n)%3); 
for(n=1, 200000, if(A070563(n)!=a(n), print1(n, ", ")) );
