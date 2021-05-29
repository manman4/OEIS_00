n = 10000
(0..n).each{|i|
  j = (i ** (i + 1) + (-1) ** (i % 2)) / (i + 1)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
