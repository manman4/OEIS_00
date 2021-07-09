def A(n)
  ary = [0]
  a, b = 0, 1
  (1..n).each{|i|
    a, b = b, (i + 1) * (b + i * a)
    ary << a
  }
  ary
end

n = 500
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
