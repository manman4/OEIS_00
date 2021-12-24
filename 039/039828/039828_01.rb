def A(n)
  a = [1]
  ary = [1]
  (1..n).each{|i|
    m = (-1) ** i
    b = a + [0] * i
    (0..a.size - 1).each{|j| b[i + j] += m * a[j]}
    a = b
    ary << a.max
  }
  ary
end

n = 1000
ary = A(n)
(1..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
