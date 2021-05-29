def A(n)
  ary = [1, 0, 0, 12]
  i = 3
  j = 6
  while i < n
    i += 1
    j *= i
    ary << ((i * i - 2 * i) * i * ary[-1] + i * i * (i - 1) * ary[-2] - 8 * (-1) ** (i % 2) * j) / (i - 2)
  end
  ary[0..n]
end

n = 300
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
