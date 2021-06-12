def A075374(n)
  a, b = 1, 2
  ary = [1]
  i = 1
  while i < n
    a, b = b, i * b - a
    ary << a
    i += 1
  end
  ary
end

n = 500
ary = A075374(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
