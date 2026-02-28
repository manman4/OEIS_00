default(parisize, 120000000)

\\ A(x)=1/(1-x-x/(1-x^2-x^2/(1-x^3-x^3/(1-x^4-x^4/(...))))).

lista(nn) = {
  my(x='x+O('x^(nn+1)), v=0);
  forstep(k=nn, 2, -1,
    v = -x^(k-1) / (1 - x^k + v)
  );
  Vec(1 / (1 - x + v))
};

M=1000;
v = lista(M);
for(n=0, M, write("b088354_02.txt", n, " ", v[n+1]));