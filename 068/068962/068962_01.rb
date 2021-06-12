n = 10000
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
  print i
  print ' '
  puts a.size
  x += 1
  x %= 6
}
