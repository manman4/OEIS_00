def A009179(n)
  a = 1
  ary = [1]
  (1..n).each{|i|
    a = (i + 1) % 2 - i * a
    ary << a
  }
  ary
end

n = 500
ary = A009179(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
