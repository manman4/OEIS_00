n = 10000000

ary = [-1, 1]
(2..n).each{|i|
  if i % 2 == 0
    ary << ary[i / 2] + ary[i / 2 - 1]
  else
    ary << (ary[i - 1] - ary[i - 2]).abs
  end
}

(1..10000).each{|i|
  j = ary.index(i)
  break if j == nil
  print i
  print ' '
  puts j
}