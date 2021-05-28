require 'prime'

n = 10000
ary = Prime.take(n + 1).map{|i| (i + 3) / 2}[1..-1]
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
