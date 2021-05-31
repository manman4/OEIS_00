def A043304(n)
  ary = []
  a = []
  (0..4 * n + 2).each{|i|
    a << 1r / (i + 1)
    i.downto(1){|j| a[j - 1] = j * (a[j - 1] - a[j])}
    ary << (a[0] / (2 * i)).denominator if i > 2 && i % 4 == 2
  }
  ary
end

n = 500
ary = A043304(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
