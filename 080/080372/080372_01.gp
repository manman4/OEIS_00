{a(n) = my(k=1); while(numdiv(k)!=n*numdiv(k+1), k++); k};
for(n=1, 20, print1(a(n), ", "))