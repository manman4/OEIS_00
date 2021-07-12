def A(k, n)
  s = 0
  (1..n).each{|i| s += i ** k if i.gcd(n) == 1}
  s
end

n = 10000
(1..n).each{|i|
  j = A(4, i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
