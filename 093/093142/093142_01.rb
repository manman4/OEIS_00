n = 1000
(0..n).each{|i|
  j = (5 * 10 ** i + 4) / 9
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
