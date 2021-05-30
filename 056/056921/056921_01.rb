def A(a0, a1, n)
  ary = [a0, a1]
  i = 0
  while ary.size < n + 1
    i += 1
    ary << ary[-1] * i + ary[-2]
    ary << ary[-1] + ary[-2]
  end
  ary[0..n]
end

n = 10000
ary = A(0, 1, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
