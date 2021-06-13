def A003687(n)
  a, b = 2, 1
  ary = [1, 2]
  while ary.size < n + 1
    a, b = a - b, a * b
    ary << a
  end
  ary[0..n]
end

n = 30
ary = A003687(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
