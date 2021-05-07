a(n) = my(s=factor(n)[, 1], k=s[#s], f=Mod(k!, n)); while(f, f*=k++); k;
A001414(n) = (n=factor(n))[, 1]~*n[, 2];

isok(n) = n==1 || a(n)==A001414(n);
M=10000;
cnt=0;
n=0;
while(cnt<M, n++; if(isok(n), cnt++; write("/Users/xxx/Desktop/b074583.txt", cnt, " ", n)))