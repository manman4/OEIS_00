def A(n)
  ary = [1, 0, 0]
  i = 2
  while i < n
    ary << i * (ary[-1] + ary[-2]) + ary[-3]
    i += 1
  end
  ary[0..n]
end

n = 500
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
