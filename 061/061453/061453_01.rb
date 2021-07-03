def f(n)
  n ** n
end

def A(n)
  a, b = 1, 1
  ary = [1]
  cnt = 0
  while cnt < n
    a, b = b, b * f(cnt + 2) + 1 * a
    ary << a
    cnt += 1
  end
  ary
end

n = 10
ary = A(n)
(1..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
