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

n = 1100
# A002893
ary = f(10, 3, -9, 3, n)
(1..n).each{|i|
  j = ary[i] / 3
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
