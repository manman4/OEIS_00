def A003487(n)
  ary = [5]
  n.times{ary << ary[-1] ** 2 - 2}
  ary
end

n = 15
ary = A003487(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
