n = 12287

ary = [-1, 1]
(2..n).each{|i|
  if i % 2 == 0
    ary << ary[i / 2] + ary[i / 2 - 1]
  else
    ary << (ary[i - 1] - ary[i - 2]).abs
  end
}

(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}