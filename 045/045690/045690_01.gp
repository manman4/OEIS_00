\\ G.f. A(x) satisfies (1-2*x)*A(x) + A(x^2) = x.
A(N) =
{
  my(x = 'x, A = 0 + O(x^(N + 1)), q = 1);
  while(q <= N,
    A = (x - subst(A, x, x^2))/(1 - 2*x) + O(x^(N + 1));
    q *= 2
  );
  A
};

A(36)
Vec(A(36))
