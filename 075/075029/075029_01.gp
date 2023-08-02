b(n) = my(k=n); while(numdiv(k)>numdiv(k+1), k++); k-n+1;
a(n) = my(k=1); while(b(k)<n, k++); k;

for(n=1, 7, print1(a(n), ", "));