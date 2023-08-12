def A(k, n)
  k ** k / n
end

n = 10000
(1..n).each{|i|
  j = A(6, i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
