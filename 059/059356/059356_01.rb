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

def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A(n)
  (f(n - 1) * (1..n - 1).inject(0){|s, i| s + sigma(1, i) * sigma(1, n - i) / i.to_r}).to_i
end

n = 1000
(2..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
} 