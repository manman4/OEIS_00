def A(k, n)
  ary = [0]
  a, mod = k, 5
  n.times{|i|
    b = a % mod
    ary << b
    a = b ** 5
    mod *= 5
  }
  ary
end

n = 1500
ary = A(2, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
