\\ A_{n,a} and A^*_{n,a} in the formula from the image.
\\ Here rf(x,k) denotes the rising factorial (x)_k.

rf(x, k) = {
  if (k < 0, error("rf: k must be nonnegative"));
  if (k == 0, return(1));
  prod(j = 0, k - 1, x + j)
};

\\ A_{n,a}; this sum is finite for every nonnegative integer n.
A(n, a) = {
  if (n < 0 || type(n) != "t_INT",
    error("A: n must be a nonnegative integer")
  );
  sum(k = 0, n,
    binomial(n, k) * (-1)^k * rf(1 - a - n - k, k) / k!
  )
};

\\ A^*_{n,a} when its infinite sum terminates.
\\ The sum terminates if q=n+a-1 is a nonnegative integer, since
\\ (1-a-n)_k=(-q)_k vanishes for k>q.
Astar_terminating(n, a) = {
  my(q = n + a - 1);
  if (n < 0 || type(n) != "t_INT",
    error("Astar_terminating: n must be a nonnegative integer")
  );
  if (type(q) != "t_INT" || q < 0,
    error("Astar_terminating: the series does not terminate")
  );
  sum(k = 0, q,
    binomial(n + k, k) * (-1)^k * rf(1 - a - n, k) / k!
  )
};

\\ For a=-1, the cases n=0,1 are nonterminating but have the exact
\\ hypergeometric value 1/4.  For n>=2 the defining sum terminates.
Astar_minus1(n) = {
  if (n < 0 || type(n) != "t_INT",
    error("Astar_minus1: n must be a nonnegative integer")
  );
  if (n <= 1, 1/4, Astar_terminating(n, -1))
};

\\ Left-hand side of the recurrence in the image.
recurrence_lhs(n, a, u0, u1, u2) = {
  (n + 2) * (n + a + 1) * (2*n + a + 1) * u2
    - (2*n + a + 2)
        * (6*n^2 + 6*a*n + 12*n + a^2 + 7*a + 4) * u1
    + (n + 1) * (n + a) * (2*n + a + 3) * u0
};

check_minus1(N = 20) = {
  my(okA = 1, okAstar = 1);
  for (n = 0, N - 2,
    if (recurrence_lhs(n, -1, A(n, -1), A(n + 1, -1), A(n + 2, -1)),
      okA = 0;
      print("A recurrence failed at n=", n)
    );
    if (recurrence_lhs(n, -1,
          Astar_minus1(n), Astar_minus1(n + 1), Astar_minus1(n + 2)),
      okAstar = 0;
      print("A* recurrence failed at n=", n)
    )
  );
  print("recurrence check for a=-1, n=0..", N - 2,
        ": A=", if(okA, "OK", "NG"),
        ", A*=", if(okAstar, "OK", "NG"));
};

\\ Output requested for a=-1.
N = 20;
print("a = -1");
print("A(n,-1), n=0..", N, ":");
print(vector(N + 1, i, A(i - 1, -1)));
print("Astar(n,-1), n=0..", N, ":");
print(vector(N + 1, i, Astar_minus1(i - 1)));
check_minus1(N);


print(vector(N + 1, i, A(i - 1, 0)));
print(vector(N + 1, i, A(i - 1, 1)));
print(vector(N + 1, i, A(i - 1, 2)));
print(vector(N + 1, i, A(i - 1, 3)));
print(vector(N + 1, i, A(i - 1, 4)));
