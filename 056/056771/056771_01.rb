def A(n)
  a, b = 1, 17
  ary = [1]
  while ary.size <= n
    a, b = b, 34 * b - a
    ary << a
  end
  ary
end

n = 600
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
