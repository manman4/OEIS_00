def s(k, m, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == m}
  s
end

def A(n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(5, 1, i) + s(5, 4, i) - s(5, 2, i) - s(5, 3, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - 5 * a[j] * ary[-j]} / i}
  ary
end

n = 41
ary = A(n - 1)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
