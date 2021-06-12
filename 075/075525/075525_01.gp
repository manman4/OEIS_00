\\ \r
\\ Nは使わない 
{a(n) = my(t='t); polcoef(n!*prod(j=1, n, (1+x^j+x*O(x^n))^t), n)};
M=100;
cnt = 1;
for(n=1, M, v=a(n); for (k=1, n, write("C:\\Users\\xxx\\Desktop\\b075525.txt", cnt, " ", polcoef(v,k)); cnt++))
