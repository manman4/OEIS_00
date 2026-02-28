default(parisize, 120000000)

\\ A(x) = 1/(1-x-x^2/(1-x^3-x^4/(1-x^5-x^6/(1-x^7-x^8/(1-...))))).

lista(nn) = {
  my(x='x+O('x^(nn+1)), v=0);
  \\ 十分な深さ (nn) から遡る
    forstep(k = nn, 2, -1,
      my(ak = -x^(2*k-2));
      my(bk = 1 - x^(2*k-1));
      v = ak / (bk + v);
    );
  Vec(1 / (1 - x + v))
};

M=1000;
v = lista(M);
for(n=0, M, write("b088352_02.txt", n, " ", v[n+1]));