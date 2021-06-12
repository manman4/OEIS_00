def A(n)
  a, b, c = 0, 0, 1
  ary = [0]
  while ary.size <= n
    a, b, c = b, c, 3 * c - 3 * b + 2 * a
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
