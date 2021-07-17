def A008747(n)
  a, b, c, d, e, f = 1, 1, 2, 3, 5, 6
  ary = [1]
  while ary.size < n + 1
    a, b, c, d, e, f = b, c, d, e, f, f + e - c - b + a
    ary << a
  end
  ary
end

n = 10000
ary = A008747(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
