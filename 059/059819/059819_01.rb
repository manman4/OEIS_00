require 'prime'

# tau(n^k)
def tau(n, k = 1)
  n.prime_division.inject(1){|s, i| s * (k * i[1] + 1)}
end

# a[0]=0
def tau_ary(n, k = 1)
  [0] + (1..n).map{|i| tau(i, k)}
end

def power0(a, n)
  return 1 if n == 0
  k = power0(a, n >> 1)
  k *= k
  return k if n & 1 == 0
  return k * a
end

# x > 0
def sigma(x, i)
  sum = 1
  pq = i.prime_division
  pq.each{|a, n| sum *= (power0(a, (n + 1) * x) - 1) / (power0(a, x) - 1)}
  sum
end

def A(n)
  t_ary = tau_ary(n)
  [0] + (1..n).map{|i| (sigma(1, i) + t_ary[i] + (1..i - 1).inject(0){|s, j| s + t_ary[j] * t_ary[i - j]}) / 2}
end

n = 10000
ary = A(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}

