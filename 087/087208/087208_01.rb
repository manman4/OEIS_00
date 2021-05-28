def A087208(n)
  a, b = 1, 1
  ary = [1]
  (1..n).each{|i|
    a, b = b, (i + 1) * i * a + 1
    ary << a
  }
  ary
end

n = 500
ary = A087208(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
