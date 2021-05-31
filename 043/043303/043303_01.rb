def A043303(n)
  ary = []
  a = []
  (0..4 * n + 2).each{|i|
    a << 1r / (i + 1)
    i.downto(1){|j| a[j - 1] = j * (a[j - 1] - a[j])}
    ary << (a[0] / (2 * i)).numerator if i % 4 == 2
  }
  ary
end

n = 160
ary = A043303(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
