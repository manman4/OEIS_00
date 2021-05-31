n = 10 ** 3
i, j = 2, 1
ary = [i, j]
while ary.max < n
  i, j = j, i + j
  ary << j
end

ps = Array.new(n + 1){0}
ps[0] = 1
ary.each{|num|
  (num..n).each{|i|
    ps[i] += ps[i - num]
  }
}
(0..n).each{|i|
  print i
  print ' '
  puts ps[i]
}
