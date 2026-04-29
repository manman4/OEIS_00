\\ PARI/GP implementation of:
\\ T(0,0) = 1
\\ T(0,k) = 0  (k > 0 or k < 0)
\\ T(n,k) = k*T(n-1,k-1) + (k+1)*T(n-1,k+1)

a104035(n, k) =
{
  my(prev, curr, step, kk, val);

  if (n < 0, error("T(n,k): n must be >= 0"));
  if (k < 0 || k > n, return(0));
  if (n == 0, return(if (k == 0, 1, 0)));

  \\ Row step stores [T(step,0), T(step,1), ..., T(step,step)].
  prev = [1];

  for (step = 1, n,
    curr = vector(step + 1);
    for (kk = 0, step,
      val = 0;
      if (kk - 1 >= 0,
        val += kk * prev[kk]
      );
      if (kk + 1 <= step - 1,
        val += (kk + 1) * prev[kk + 2]
      );
      curr[kk + 1] = val;
    );
    prev = curr;
  );

  prev[k + 1]
};

M=482;
\\ for(n=0, 10, for(k=0, n, print1(a104035(n, k), ", ")));
\\ a(n) = A104035(n,2) for even n, and a(n) = A104035(n,3) for odd n. 
a(n) = if(n%2==0, a104035(n, 2), a104035(n, 3));
for(n=1, M, write("b002435.txt", n, " ", a(n)))
