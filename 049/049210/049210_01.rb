n = 400
s = -1
(0..n).each{|i|
  break if s.to_s.size > 1000
  print i
  print ' '
  puts -s
  s *= (8 * i + 7)
}
