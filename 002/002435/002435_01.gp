\\ PARI/GP translation of the given Maple code.
\\ We work with power series around x = 0 and extract constant terms.

default(seriesprecision, 96);

nthderiv(f, n) =
{
  my(g = f);
  for (i = 1, n, g = deriv(g));
  g
};

p(n) =
{
  my(N = 96, x, F, z, w, dn, base, expr, sgn);

  x = 'x + O('x^N);
  F = 2 / (exp(x) + exp(-x));
  z = ((exp(x) - exp(-x)) / (exp(x) + exp(-x)))^2;
  w = deriv(z);
  dn = nthderiv(F, n);
  sgn = (-1)^(1 + n \ 2);

  if (n % 2 == 0,
    base = dn / F,
    base = dn / deriv(F)
  );

  expr = deriv(base) / w;
  sgn * polcoeff(expr, 0)
};

for(n=1, 28, print1(p(n),", "));
