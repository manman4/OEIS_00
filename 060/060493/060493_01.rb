def A(n)
  (1..n).inject(0){|s, i| s + i ** 3 * (i + 1) * (2 * i + 1) / 6}
end

n = 10000
(0..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}