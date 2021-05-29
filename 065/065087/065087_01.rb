def A(n)
  ary = [0, 0]
  cnt = 1
  a = 0
  while cnt < n
    cnt += 1
    a = cnt * (cnt + 1) * (a / (cnt - 1r) + (-1) ** (cnt % 2) / 2r)
    ary << a
  end
  ary[0..n]
end

n = 400
ary = A(n)
(0..n).each{|i|
  j = ary[i].to_i
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
