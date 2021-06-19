def A(n)
  return 1 if n == 0
  (2 * (-4) ** n + 16 ** n) / 4
end

n = 1000
(0..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
