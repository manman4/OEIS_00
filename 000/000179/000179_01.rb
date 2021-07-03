def A(n)
  # a[1] = -1
  ary = [1, -1, 0, 1]
  i = 3
  while i < n
    i += 1
    ary << ((i * i - 2 * i) * ary[-1] + i * ary[-2] - 4 * (-1) ** (i % 2)) / (i - 2)
  end
  ary[0..n]
end

n = 25
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
