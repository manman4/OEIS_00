def A(n)
  ary = [1, 2]
  (2..n).each{|i| ary << i * i * ary[-1] - i * (i - 1) * ary[-2] + 1}
  ary
end

n = 1000
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
