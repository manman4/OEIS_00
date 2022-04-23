n = 1000
(1..n).each{|i|
  j = (9 ** (i + 1) - 8 * i - 9)/64
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
