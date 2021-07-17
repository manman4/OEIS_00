n = 400
s = 1
a, b, c, d = -5, -3, -1, 0
print 0
print ' '
puts 1
(1..n).each{|i|
  a += 6
  b += 6
  c += 6
  d += 1
  s *= a * b * c * 8r / (d * d * d)
  break if (s.to_i).to_s.size > 1000
  print i
  print ' '
  puts s.to_i
}
