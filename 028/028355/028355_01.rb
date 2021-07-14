n = 3000
ary = [1, 2, 3, 4, 3, 2]
x = 0
(1..n).each{|i|
  s = ary[x]
  a = [s]
  while s < i
    x += 1
    x %= 6
    s += ary[x]
    a << ary[x]
  end
  j = a.join
  break if j.size > 1000
  print i
  print ' '
  puts j.to_i
  x += 1
  x %= 6
}
