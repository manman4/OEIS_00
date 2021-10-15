require 'prime'

n = 10000
p_ary = [0] + Prime.take(n).to_a
(1..n).each{|i|
  j = p_ary[i]
  k = 1
  while !(j + k * i).prime?
    k += 1
  end
  print i
  print ' '
  puts k
}
