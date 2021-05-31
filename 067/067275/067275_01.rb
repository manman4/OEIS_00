def A(n)
  return 0 if n == 0
  return 1 if n == 1
  ([6] * (n - 2) + [7]).join.to_i
end

n = 1010
(0..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
