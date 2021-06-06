def A077855(n)
  a, b, c, d = 1, 3, 6, 11
  ary = [1]
  while ary.size < n + 1
    a, b, c, d  = b, c, d, 3 * d - 3 * c + 2 * b - a
    ary << a
  end
  ary
end

n = 5000
ary = A077855(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
