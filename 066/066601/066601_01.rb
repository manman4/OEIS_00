def pow(a, m, mod)
  return 1 if m == 0
  k = pow(a, m >> 1, mod)
  k *= k
  return k % mod if m & 1 == 0
  return k * a % mod
end

def A(k, n)
  (1..n).map{|i| pow(k, i, i)}
end

n = 10000
ary = A(3, n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
