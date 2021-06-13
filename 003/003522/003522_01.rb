def A(n)
  a, b, c, d = 1, 1, 1, 1
  ary = [1]
  while ary.size <= n
    a, b, c, d = b, c, d, 3 * d - 3 * c + b + a
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
