def A(k, m, n)
  ary = [0]
  a, mod = k, m
  while ary.size - 1 < n
    b = a % mod
    ary << b if b != ary[-1]
    a = b ** m
    mod *= m
  end
  ary
end
def A034935(n)
  A(2, 5, n)
end
n = 20
ary = A034935(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
