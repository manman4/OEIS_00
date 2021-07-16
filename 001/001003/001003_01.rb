def A001003(n)
  ary = [1]
  i = 0
  a, b = 1, 1
  while i < n
    a, b = b, ((6 * i + 9) * b - i * a) / (i + 3)
    ary << a
    i += 1
  end
  ary
end

n = 1500
ary = A001003(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
