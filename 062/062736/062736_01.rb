require 'prime'

p_ary = Prime.take(2 * 10 ** 5).to_a
ary = []
s = 0
p_ary.each{|i|
  s += i + 1
  if s.prime?
    ary << s
  end
}
(1..10000).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
