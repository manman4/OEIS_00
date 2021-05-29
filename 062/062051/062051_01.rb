n = 10 ** 3
ary = (0..Math.log(n, 3).to_i).map{|i| 3 ** i}

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
