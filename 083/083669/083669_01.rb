n = 10000
(0..n).each{|i|
  j = 1 + 5 * i * (i + 1) * (23 * i * i + 23 * i + 14) / 12
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
