require 'prime'

n = 10000
ary = [0] + Prime.take(2 * n).to_a
(1..n).each{|i|
  j = ary[2 * i] + ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
