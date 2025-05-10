n = 17

def a_row(n): return [(((1+sqrt(2))/1)^(k*(n-k))*q_binomial(n, k, -((3-2*sqrt(2)))^1)).simplify_full() for k in (0..n)]
for n in (0..9): print(a_row(n))