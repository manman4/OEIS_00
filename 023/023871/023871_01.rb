require 'prime'

def power0(a, n)
  return 1 if n == 0
  k = power0(a, n >> 1)
  k *= k
  return k if n & 1 == 0
  return k * a
end

def sigma(x, i)
  sum = 1
  pq = i.prime_division
  if x == 0
    pq.each{|a, n| sum *= n + 1}
  else
    pq.each{|a, n| sum *= (power0(a, (n + 1) * x) - 1) / (power0(a, x) - 1)}
  end
  sum
end

def A(k, n)
  ary = [1]
  s_ary = [0] + (1..n).map{|i| sigma(k, i)}
  n.times{|i|
    ary << (1..i + 1).inject(0){|s, j| s + ary[-j] * s_ary[j]} / (i + 1)
  }
  ary
end

n = 31
ary = A(3, n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
