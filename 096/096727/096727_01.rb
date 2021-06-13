def s(k, m, n)
  s = 0
  (1..n).each{|i| s += (n / i) ** m if n % i == 0 && i % k > 0}
  s
end

def A(k, m, n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(k, m, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - k * k * k * a[j] * ary[-j]} / i}
  ary
end

n = 56
ary = A(2, 1, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
