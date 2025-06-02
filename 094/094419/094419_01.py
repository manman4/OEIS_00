from mpmath import polylog

N = 9
# a(n) = (-1)^(n+1)/7 * Li_{-n}(7/6), where Li_{n}(x) is the polylogarithm function.
print([int(round((-1)**(n+1) / 7 * polylog(-n, 7/6))) for n in range(N + 1)])
