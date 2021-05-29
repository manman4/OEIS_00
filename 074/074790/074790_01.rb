def A074790(n)
  a = 1
  ary = [1]
  (1..n).each{|i|
    a = 2 * i * (2 * i + 1) * a + (-1) ** i
    ary << a
  }
  ary
end

n = 250
ary = A074790(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
