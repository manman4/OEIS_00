{a(n) = my(k=1); while(numdiv(k+1)!=n*numdiv(k), k++); k};
for(n=1, 20, print1(a(n), ", "))