def A(n)
  a, b, c, d = 4, 0, 0, 3
  ary = [4]
  while ary.size <= n
    a, b, c, d = b, c, d, a + b
    ary << a
  end
  ary
end

n = 10000
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
