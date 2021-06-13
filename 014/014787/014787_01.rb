require 'prime'

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

def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == 0}
  s
end

def A(n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(1, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - 12 * a[j] * ary[-j]} / i}
  ary
end

n = 33
a = [0] + (1..2 * n + 3).map{|i| sigma(5, i)}
b = A(n + 1)
(0..n).each{|i|
  print i
  print ' '
  puts (a[2 * i + 3] - b[i + 1]) / 256
}
p (0..n).map{|i| (a[2 * i + 3] - b[i + 1]) / 256}
