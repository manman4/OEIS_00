M=100;

E(n, k) = sum(j=0, k, (-1)^j*binomial(n+1, j)*(k+1-j)^n);
a271697(n, k) = sum(j=0, n, (-1)^(n-j)*binomial(n, j)*E(j, k));
T(n, k) = sum(e=0, (n+k)\2, binomial(n, 2*e-k)*a271697(2*e-k, e)); 
cnt=0;
\\ n=0,1のときはk=0のみ出力 n>=2のときはk=-(n-2)からn-2まで出力
b(n) = if(n<2, 0, n-2);
\\ for(n=0, M, for(k=-b(n), b(n), write("b062866_1.txt", cnt, " ", T(n, k)); cnt++));
for(n=0, 10, for(k=-b(n), b(n), print1(T(n, k),", ")));