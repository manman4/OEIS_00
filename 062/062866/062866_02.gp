M=10;

E(n, k) = sum(j=0, k, (-1)^j*binomial(n+1, j)*(k+1-j)^n);
a271697(n, k) = sum(j=0, n, (-1)^(n-j)*binomial(n, j)*E(j, k));
\\ T(n,k) = Sum_{f,d,e>=0, f+d+e=n and e-d=k} binomial(n,f) * A271697(d+e,e)
T(n, k) = {
  my(s = 0);
  for(f=0, n,
    for(d=0, n-f,
      my(e = n - f - d);
      if(e>=0 && e-d==k, s += binomial(n, f)*a271697(d+e, e));
    );
  );
  s
};

\\ n=0,1のときはk=0のみ出力 n>=2のときはk=-(n-2)からn-2まで出力
b(n) = if(n<2, 0, n-2);
for(n=0, M, for(k=-b(n), b(n), print1(T(n, k),", ")));