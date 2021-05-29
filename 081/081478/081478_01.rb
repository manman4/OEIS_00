def A081478(n)
  a, b = 2, 1
  ary = [1]
  while ary.size < n
    a, b = a - b, a * b
    ary << b
  end
  ary
end

n = 30
ary = A081478(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
