def d(n, m): return not n % m
def A007678(n): return (1176*d(n, 12)*n - 3744*d(n, 120)*n + 1536*d(n, 18)*n - d(n, 2)*(5*n**3 - 42*n**2 + 40*n + 48) - 2304*d(n, 210)*n + 912*d(n, 24)*n - 1728*d(n, 30)*n - 36*d(n, 4)*n - 2400*d(n, 42)*n - 4*d(n, 6)*n*(53*n - 310) - 9120*d(n, 60)*n - 3744*d(n, 84)*n - 2304*d(n, 90)*n + 2*n**4 - 12*n**3 + 46*n**2 - 84*n)//48 + 1 

n = 10000
for i in range(1, n+1):
    print(i, A007678(i))
