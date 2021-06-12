def A051277(n)
  d_ary = []
  ary = [0]
  a, mod = 3, 7
  (n + 1).times{|i|
    b = a % mod
    d_ary << (b - ary[-1]) / 7 ** i
    ary << b
    a = b * b + b - 2
    mod *= 7
  }
  d_ary
end
n = 10000
ary = A051277(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
