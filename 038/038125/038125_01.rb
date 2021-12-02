def A(k, n)
  (0..n).inject(0){|s, i| s + (i - n) ** (i + k)}
end

n = 500
(0..n).each{|i|
  j = A(0, i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
