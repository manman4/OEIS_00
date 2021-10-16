isok(k) = my(d=digits(k^2, 6)); d == Vecrev(d);
k=0;
cnt=0;
while(cnt<79, if(isok(k), cnt++; write("/Users/xxx/Desktop/b029990.txt", cnt, " ", k)); k++);