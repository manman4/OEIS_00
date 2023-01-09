def A(n)
  s = 0
  (0..n).each{|i|
    s = [s, i ** (n - i)].max
  }
  s
end

n = 1000
(0..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}