def A(n)
  s = 1
  t = 1
  (1..n).each{|i|
    t *= n / i
    s += t
  }
  s
end


n = 500
(0..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
