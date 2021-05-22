def A(n)
  a, b = 0, 1
  ary = [0]
  s = 0
  while ary.size <= n
    if s - a > 0
      s -= a
    else
      s += a
    end
    ary << s
    a, b = b, a + b
  end
  ary
end

n = 4000
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
