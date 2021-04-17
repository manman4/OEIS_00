def A(k, n)
  (1..n).inject(0){|s, i| s + k ** (i.gcd(n) - 1)}
end

n = 5000
(1..n).each{|i|
  j = A(3, i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}