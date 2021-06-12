def A051396(n)
  a = 0
  ary = [0]
  (1..n).each{|i|
    a = (2 * i - 2) * (2 * i - 3) * a + 1
    ary << a
  }
  ary
end

n = 250
ary = A051396(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
