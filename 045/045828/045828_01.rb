def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == 0}
  s
end

def A(n)
  ary = [1]
  a = [0] + (1..n).map{|i| 3 * s(2, i) + 2 * s(4, i) - 2 * s(1, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  ary
end

n = 84
ary = A(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
