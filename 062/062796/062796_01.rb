def s(i)
  s = 0
  (1..i).each{|j| s += j ** j if i % j == 0}
  s
end

n = 500
ary = (1..n).map{|i| s(i)}
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
