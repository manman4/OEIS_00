n = 500
s = 0
(0..n).each{|i|
  s *= i * i
  s += (-1) ** i
  break if s.to_s.size > 1000
  print i
  print ' '
  puts s
}
