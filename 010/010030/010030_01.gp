main() = {
  my(N = 150, x = 'x, q = 'q);
  my(num, den, db_num, a, c, r, ac, cc, rc, g, s);
  my(out, cnt = 1, M, runs, v);

  \\ Put q = 1-y and write B = num/den.
  \\ G(x,q) = Sum_{m>=0} m!*B^m is first computed in the q-basis.
  num = x*(1-2*q*x+q*x^2);
  den = 1-q*x^2;
  db_num = deriv(num, x)*den-num*deriv(den, x);

  \\ If F(z) = Sum_{m>=0} m!*z^m, then
  \\ z^2*F'(z) + (z-1)*F(z) + 1 = 0.
  \\ Substituting z = B gives
  \\ a*G_x + (c-1)*G + r = 0, hence G = a*G_x + c*G + r.
  a = num^2*den;
  c = (num-den)*db_num+1;
  r = db_num*den;

  \\ Coefficients of the fixed, low-degree polynomials in x.
  ac = vector(9, i, polcoef(a, i-1, x));
  cc = vector(8, i, polcoef(c, i-1, x));
  rc = vector(7, i, polcoef(r, i-1, x));

  \\ g[n+1] = [x^n]G(x,q), as a polynomial in q.
  g = vector(N+1);
  g[1] = 1;
  for(n = 1, N,
    s = if(n <= 6, rc[n+1], 0);
    for(i = 2, min(8, n),
      s += ac[i+1]*(n-i+1)*g[n-i+2];
    );
    for(i = 1, min(7, n),
      s += cc[i+1]*g[n-i+1];
    );
    g[n+1] = s;
  );

  \\ [y^runs]q^h = (-1)^runs*binomial(h,runs), q = 1-y.
  \\ A010030 lists the run numbers floor(n/2),...,0 and divides by 2.
  out = fileopen("b010030_1.txt", "w");
  for(n = 1, N,
    M = n\2;
    for(k = 0, M,
      runs = M-k;
      s = (-1)^runs*sum(h = runs, M,
        polcoef(g[n+1], h, q)*binomial(h, runs)
      );
      v = if(n == 1, 1, s/2);
      filewrite(out, Str(cnt, " ", v));
      cnt++;
    );
  );
  fileclose(out);
}

main();
