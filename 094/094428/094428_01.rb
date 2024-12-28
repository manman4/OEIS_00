def s(k, m, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == m}
  s
end

def A(n)
  ary = [1]
  a = [0] + (1..n).map{|i| 2 * s(12, 0, i) + s(12, 6, i) - s(12, 5, i) - s(12, 7, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  ary
end

n = 10100
m = 10000
ary = A(n)
(0..m).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
