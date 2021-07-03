def A(n)
  ary = [1]
  i = 0
  a, b = 1, -1
  while i < n
    i += 1
    a, b = b, -b - 2 * i * a
    ary << a
  end
  ary
end

n = 1000
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
