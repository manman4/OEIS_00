def A(n)
  a, b, c, d, e, f = 1, -7, 28, -84, 210, -462
  ary = [1]
  while ary.size <= n
    a, b, c, d, e, f = b, c, d, e, f, -7 * f - 21 * e - 35 * d - 35 * c - 21 * b - 7 * a
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
