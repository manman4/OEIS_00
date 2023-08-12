def A(k, n)
  k ** (n - 1) / n
end

n = 1000
(1..n).each{|i|
  j = A(10, i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
