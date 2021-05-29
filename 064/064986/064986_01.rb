n = 10000
ary = [1] + (2..20).map{|i| (1..i).inject(:*)}
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
