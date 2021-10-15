def A(k, m, n)
  ary = [1]
  (1..n).each{|i|
    ary << (0..k * (i - 1) / m).inject(0){|s, j| s + ary[j] * ary[i - 1 - j]}
  }
  ary
end

n = 1000
ary = A(4, 5, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
