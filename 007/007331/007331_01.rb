def s(k, m, n)
  s = 0
  (1..n).each{|i| s += (n / i) ** m if n % i == 0 && i % k > 0}
  s
end

def A(k, m, n)
  [0] + (1..n).map{|i| s(k, m, i)}
end

n = 40
ary = A(2, 3, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
