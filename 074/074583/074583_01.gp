isok(n) = my(f=factor(n)); n==1 || (#f~==1 & f[1, 1]>=f[1, 2]); 
M=10000;
cnt=0;
n=0;
while(cnt<M, n++; if(isok(n), cnt++; write("/Users/xxx/Desktop/b074583_1.txt", cnt, " ", n)))