default(parisize, 120000000)

\\ 1/(1 - 2*d/(1 - d^2/(1 - (d^2 + d^3)/(1 - d^4/(1 - (d^3 + d^5)/(1 - d^6/( (...) ))))))).

lista_complex(nn) = {
  my(d = 'd + O('d^(nn+1)), v = 0);
  \\ 十分な深さから計算を開始（精度の2倍程度）
  forstep(k = nn*2, 2, -1,
    my(ak);
    if(k == 1, ak = 1,
      if(k == 2, ak = -2*d,
        if(k % 2 == 1, 
          ak = -d^(k-1),             \\ 奇数番目
          ak = -(d^(k/2) + d^(k-1))  \\ 偶数番目
        )
      )
    );
    v = ak / (1 + v);
  );
  Vec(1 / (1 + v));
};

M=1000;
v = lista_complex(M);
for(n=0, M, write("b088354_03.txt", n, " ", v[n+1]));