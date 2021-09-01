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

def A000041(n)
  ary = Array.new(n + 1, 1)
  A([[ary, ary]], n)
end

n = 2000
ary = A000041(2 * n + 1)
(0..n).each{|i|
  j = ary[2 * i + 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}