def A(m, n)
  ary = [m]
  a, b = m, 1
  (1..n).each{|i|
    a, b = b, i * a + b
    ary << a
  }
  ary
end

n = 26
ary = A(0, n + 1)
(0..n).each{|i|
  j = ary[i + 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
