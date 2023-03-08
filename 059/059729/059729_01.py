def A059729(n):
    s = [int(d) for d in str(n)]
    l = len(s)
    t = [0]*(2*l-1)
    for i in range(l):
        for j in range(l):
            t[i+j] = (t[i+j] + s[i]*s[j]) % 10
    return int("".join(str(d) for d in t)) 

n = 9999
for i in range(n + 1):
    print(f'{i} {A059729(i)}')
