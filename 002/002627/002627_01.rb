n = 500
s = 0
(0..n).each{|i|
  break if s.to_s.size > 1000
  print i
  print ' '
  puts s
  s *= i + 1
  s += 1
}
