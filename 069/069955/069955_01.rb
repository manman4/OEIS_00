def A069955(n)
  s = 1
  d_ary = [1]
  (1..n).each{|i|
    s *= (2 * i) * (2 * i) / ((2 * i - 1r) * (2 * i + 1))
    d_ary << s.denominator
  }
  d_ary
end

n = 900
ary = A069955(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
