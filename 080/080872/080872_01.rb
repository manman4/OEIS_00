def A(n)
  a, b, c = 1, 1, 5
  ary = [1]
  while ary.size <= n
    a, b, c = b, c, (4 + b * c) / a
    ary << a
  end
  ary
end

n = 2000
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
