def A034945(n)
  ary = [0]
  a, mod = 3, 7
  while ary.size - 1 < n
    b = a % mod
    ary << b if b != ary[-1]
    a = b * b + b - 2
    mod *= 7
  end
  ary
end
p A034945(19)
# (0..n).each{|i|
#   j = ary[i]
#   break if j.to_s.size > 1000
#   print i
#   print ' '
#   puts j
# }
