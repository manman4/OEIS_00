def A(k, m, n)
  a, b = 0, 1
  ary = [0]
  cnt = 0
  while cnt < n
    a, b = b, 2 * m * b / (cnt + 1) + k * k * a
    ary << a
    cnt += 1
  end
  ary
end

n = 10000
ary = A(1, 7, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
