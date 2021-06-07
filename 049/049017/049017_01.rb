def A(n)
  a, b, c, d, e, f, g = 1, 7, 28, 84, 210, 462, 924
  ary = [1]
  while ary.size <= n
    a, b, c, d, e, f, g = b, c, d, e, f, g, 7 * g - 21 * f + 35 * e - 35 * d + 21 * c - 7 * b + 2 * a
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
