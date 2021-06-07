n = 1000
m = 7 * n + 5

ps = Array.new(m + 1){0}
ps[0] = 1
(1..m).each{|num|
  (num..m).each{|i|
    ps[i] += ps[i - num]
  }
}

(0..n).each{|i|
  j = ps[7 * i + 5] / 7
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
