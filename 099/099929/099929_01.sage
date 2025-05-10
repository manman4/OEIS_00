n = 17

def a(n): return ((1+sqrt(2))^n^2*q_binomial(2*n, n, -(3-2*sqrt(2)))).simplify_full()
print([a(n) for n in range(n + 1)])
