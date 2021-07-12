def A(k, n)
  ary = [1]
  s = 1
  (0..n - 1).each{|i|
    s *= (k * i + 1) * k
    s /= (i + 1)
    ary << s
  }
  ary
end

n = 500
ary = A(4, n)
(0..n).each{|i|
  j = ary[i].to_i
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
