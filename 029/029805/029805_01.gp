isok(k) = my(d=digits(k^2, 8)); d == Vecrev(d);
k=0;
cnt=0;
while(cnt<100, if(isok(k), cnt++; write("/Users/xxx/Desktop/b029805_1.txt", cnt, " ", k)); k++);