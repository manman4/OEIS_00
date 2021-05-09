def A(k, n)
  a, b = 1, k - 1
  ary = [1]
  while ary.size <= n
    a, b = b, k * b - a
    ary << a
  end
  ary
end

n = 20
p ary = A(678, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
