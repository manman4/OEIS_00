\\ T(n, k) = T_2(n, k) where T_2(0, k) = 1, T_2(n, k) = Sum_{i=0..n-1} (-1)^(n-i-1)*binomial(n, i)*(i+k)^(2*(n-i))*T_2(i, k), n > 0.
T(n, k) = if(n==0, 1, sum(i=0, n-1, (-1)^(n-i-1)*binomial(n, i)*(i+k)^(2*(n-i))*T(i, k)));
for(n=0, 10, for(k=0, n-1, print1(T(k, n-k),", ")));

\\ log(1+k^2*x) = Sum_{j>=1} T(j,k)/j * (x/(1 + (j+k)^2*x))^j.
for(k=1, 5, my(N=15, x='x+O('x^N)); print(sum(j=1, N, T(j,k)/j * (x/(1+(j+k)^2*x))^j) - log(1+k^2*x)));

\\ 1 = Sum_{j>=0} T(j,k) * binomial(j+m-1,j) * x^j/(1 + (j+k)^2*x)^(j+m) for m >= 1.
for(k=1, 5, for(m=1, 5, my(N=15, x='x+O('x^N)); print(sum(j=0, N, T(j,k) * binomial(j+m-1,j) * x^j/(1+(j+k)^2*x)^(j+m)))));

\\ 1 = Sum_{j>=0} T(j,k) * x^j/j! * exp(-(j+k)^2*x).
for(k=1, 5, my(N=15, x='x+O('x^N)); print(sum(j=0, N, T(j,k) * x^j/j! * exp(-(j+k)^2*x))));

