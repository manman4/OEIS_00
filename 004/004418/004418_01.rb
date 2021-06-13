def s(k, m, n)
  s = 0
  (1..n).each{|i| s += (n / i) ** m if n % i == 0 && i % k > 0}
  s
end

def A(k, n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(2, 1, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s + 2 * k * a[j] * ary[-j]} / i}
  ary
end

n = 10000
ary = A(17, n)
(0..n).each{|i|
  j = (-1) ** (i % 2) * ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
