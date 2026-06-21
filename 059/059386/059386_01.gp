\\ a(0) = 1; a(n) = Sum_{i=1..floor(n/2)} 1/(2^(2*i-1)*(2*i)!) * Sum_{j=1..2*i} (-1)^j * j^(2*n) * binomial(4*i,2*i-j).
a(n) = if(n==0, 1, sum(i=1, n\2, 1/(2^(2*i-1)*(2*i)!) * sum(j=1, 2*i, (-1)^j * j^(2*n) * binomial(4*i, 2*i-j))));
for(n=0, 15, print1(a(n), ", "));

