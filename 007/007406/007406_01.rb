n = 2000
s = 0
(1..n).each{|i|
  s += 1r / (i * i)
  j = s.numerator
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
