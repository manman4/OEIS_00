def A(n)
  a, b, c = 1, 1, 1
  ary = [1, 1]
  while ary.size <= n
    a, b, c = b, c, 4 * c - 6 * b + 4 * a
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
