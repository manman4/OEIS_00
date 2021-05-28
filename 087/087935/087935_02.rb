def A(n)
  a, b, c, d, e = 5, 0, 0, 0, 4
  ary = [5]
  while ary.size <= n
    a, b, c, d, e = b, c, d, e, a + b
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
