def A(n)
  a, b = 1, 2
  ary = [1]
  while ary.size <= n
    a, b = b, 2 * b - 2 * a
    ary << a
  end
  ary
end

n = 5000
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
