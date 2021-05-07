isok(n) = my(f=factor(n)); n==1 || (#f~==1 & f[1, 1]>=f[1, 2]); 
for(n=1, 1000, if(isok(n), print1(n, ", ")));