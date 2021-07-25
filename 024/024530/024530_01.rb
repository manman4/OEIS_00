require 'prime'

s = 0
n = 1000
p_ary = Prime.take(n).to_a
print 0
print ' '
puts 0
(1..n).each{|i|
  s += (-1) ** ((i + 1) % 2) * 1r / p_ary[i - 1]
  j = s.numerator
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
