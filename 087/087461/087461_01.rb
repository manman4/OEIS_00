require 'prime'

n = 10000
ary = [0] + Prime.take(2 * n).to_a
(2..n).each{|i|
  j = (ary[2 * i] + ary[i]) / 2
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
