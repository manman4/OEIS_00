a(n) = if(n==0, 1, sumdiv(n, d, n^omega(d))); 
M=10000;
for(n=0, M, write("/Users/xxx/Desktop/b062319_1.txt", n, " ", a(n)))