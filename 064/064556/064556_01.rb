# n > 1
def f(n, p, t)
  k = p ** 11
  ary = [1, t]
  a, b = 1, t
  while ary.size < n
    a, b = b, t * b - k * a
    ary << b
  end
  ary
end

n = 200
ary2 = f(n + 1, 2, -24)
ary5 = f(n + 1, 5, 4830)
(0..n).each{|i|
  j = ary2[i] * ary5[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
