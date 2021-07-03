def A(n)
  (0..n - 1).inject(0){|s, i| s + i * (n - i) ** (n - i)}
end

n = 600
(1..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
