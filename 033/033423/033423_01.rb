def A(k, n)
  10 ** k / n
end

n = 10000
(1..n).each{|i|
  j = A(9, i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
