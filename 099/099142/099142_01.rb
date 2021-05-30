def A(k, n)
  a, b = 1, k + 1
  ary = [1]
  while ary.size <= n
    a, b = b, 2 * (k + 1) * b - (k - 1) ** 2 * a
    ary << a
  end
  ary
end
n = 50
ary = A(7, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
