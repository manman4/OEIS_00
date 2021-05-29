def f(a, b, c, s, n)
  x, y, m = 1, s, 0
  ary = [1]
  while m < n
    m += 1
    x, y = y, ((a * m * m + a * m + b) * y + c * m * m * x) / ((m + 1) * (m + 1))
    ary << x
  end
  ary
end

n = 1200
ary = f(12, 4, -32, 4, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
