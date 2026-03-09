# m>0
def s(f_ary, g_ary, n, m)
  s = 0
  (m..n).each{|i| s += i * f_ary[i] * g_ary[i] ** (n / i) if n % i == 0}
  s
end

def A(ary, n, m = 1)
  a_ary = [1]
  a = [0] + (1..n).map{|i| ary.inject(0){|s, j| s + s(j[0], j[1], i, m)}}
  (1..n).each{|i| a_ary << (1..i).inject(0){|s, j| s + a[j] * a_ary[-j]} / i}
  a_ary
end

def a083710(n)
  ary1 = Array.new(n + 1, 1)
  # Product_{k>0} 1/(1 - x^k)
  p_ary = A([[ary1, ary1]], n)

  ary = [1]
  # a(n) = Sum_{d|n} p(d-1)
  (1..n).each{|i| ary << (1..i).inject(0){|sum, d| sum + (i % d == 0 ? p_ary[d - 1] : 0)}}
  ary
end

n = 10100
m = 10000
digit_limit = 1000
ary = a083710(n)
(0..m).each{|i|
  j = ary[i]
  break if j.to_s.size > digit_limit
  print i
  print ' '
  puts j
}
