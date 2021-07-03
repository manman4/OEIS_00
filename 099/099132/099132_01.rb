def A(n)
  a, b, c, d, e, f = 1, 1, 1, 1, 1, 1
  ary = [1]
  while ary.size <= n
    a, b, c, d, e, f = b, c, d, e, f, 5 * f - 10 * e + 10 * d - 5 * c + b + a
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
