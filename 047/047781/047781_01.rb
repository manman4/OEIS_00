def S(k, m, n)
  a, b = 1, 2 * m
  ary = [1]
  cnt = 0
  while cnt < n
    a, b = b, (2 * m * b + k * k * cnt * a) / (cnt + 2r)
    a = a.to_i if a.denominator == 1
    ary << a
    cnt += 1
  end
  ary
end

def A(m, n)
  S(1, m, n)
end

n = 1000
print 0
print ' '
puts 0
(1..n).each{|i|
  j = A(i, i)[-1] / 2
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
