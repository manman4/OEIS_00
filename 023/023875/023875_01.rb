require 'prime'
def power(a, n)
  return 1 if n == 0
  k = power(a, n >> 1)
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
    pq.each{|a, n| sum *= (power(a, (n + 1) * x) - 1) / (power(a, x) - 1)}
  end
  sum
end
def A(k, m, n)
  ary = [1]
  s_ary = [0] + (1..n).map{|i| sigma(k, i * m)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s + ary[-j] * s_ary[j]} / i}
  ary
end

n = 20
ary = A(7, 1, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
