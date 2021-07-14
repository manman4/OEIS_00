def A028365(n)
  ary = [1]
  a, b, c = 1, 2, 24
  while ary.size < n + 1
    a, b, c = b, c, (6 * a * c * c - 8 * b * b * c) / (a * b)
    ary << a
  end
  ary
end

n = 100
ary = A028365(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
