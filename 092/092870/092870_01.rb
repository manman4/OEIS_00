n = 500
s = 1
a, b, c = -11, -7, 0
print 0
print ' '
puts 1
(1..n).each{|i|
  a += 12
  b += 12
  c += 1
  s *= a * b * 12r / (c * c)
  break if (s.to_i).to_s.size > 1000
  print i
  print ' '
  puts s.to_i
}
