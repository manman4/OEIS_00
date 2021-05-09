n = 1010
(0..n).each{|i|
  j = (10 ** i + 2) / 3
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
