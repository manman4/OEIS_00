m = 2
n = 300
s = 1
sum = 1
print 0
print ' '
puts sum
(1..n).each{|i|
  s *= i
  sum += s ** m
  break if sum.to_s.size > 1000
  print i
  print ' '
  puts sum
}
