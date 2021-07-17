print 0
print ' '
puts 1
s = 1
(1..10000).each{|i|
  s *= i
  print i
  print ' '
  puts s.to_s[0].to_i
}
