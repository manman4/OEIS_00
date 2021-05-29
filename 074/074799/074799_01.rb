def A(n)
  s = 1
  ary = [1]
  n.times{|i|
    s *= ((2 * i + 1r) / (2 * i + 2)) ** 5
    ary << s * (4 * i + 5)
  }
  ary
end

n = 400
ary = A(n)
(0..n).each{|i|
  j = ary[i].numerator
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
