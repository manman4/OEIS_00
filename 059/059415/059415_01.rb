def A059415(n)
  i = 0
  a, b = 0, 6
  ary = [0]
  while i < n
    i += 1
    a, b = b, ((((34 * i + 51) * i + 27) * i + 5) * b - i ** 3 * a) / (i + 1r) ** 3
    ary << a.numerator
  end
  ary
end

n = 1000
ary = A059415(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
