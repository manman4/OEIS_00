def S(s, t, n)
  a, b = 1, s
  ary = [1]
  cnt = 1
  while cnt <= n - 1
    a, b = b, s * b * (cnt + 1) + t * a
    ary << a
    cnt += 1
  end
  ary
end

n = 500
ary = S(2, -1, n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
