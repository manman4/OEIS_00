cnt=0;
M=10000;
for(k=1, 1e8, if(isok(k), cnt++; write("/Users/xxx/Desktop/b073543_1.txt", cnt, " ", k); if(cnt>=M, break)))