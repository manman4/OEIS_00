def A(k, n)
  (n ** k - (-1) ** (k % 2)) / (n + 1)
end

n = 1000
(0..n).each{|i|
  j = A(i, i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
