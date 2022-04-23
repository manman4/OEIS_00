def A(k, n)
  (1..n).to_a.join.to_i.to_s(k)
end

n = 1000
(1..n).each{|i|
  j = A(2, i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}