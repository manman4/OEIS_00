def A009628(n)
  a = 0
  (0..n).map{|i| a = -i * a + i % 2}
end

n = 20
ary = A009628(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
