M=20;

a(n) = (-1)^n*(prod(k=0, n-1, I+k)+prod(k=0, n-1, -I+k))/2;
for(n=0, M, print1(a(n), ", "));