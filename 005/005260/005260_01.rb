def f(n)
  x, y, m = 1, 2, 0
  ary = [1]
  while m < n
    m += 1
    x, y = y, ((2 * m + 1) * (6 * m * m + 6 * m + 2) * y + m *(64 * m * m - 4) * x) / ((m + 1) ** 3)
    ary << x
  end
  ary
end

n = 1000
ary = f(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
