def A(n)
  a, b, c, d = 1, -5, 15, -35
  ary = [1]
  while ary.size <= n
    a, b, c, d = b, c, d, -5 * d - 10 * c - 10 * b - 5 * a
    ary << a
  end
  ary
end

n = 3000
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
