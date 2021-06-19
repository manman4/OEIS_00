def A(n)
  return [0] if n == 0
  ary = [0, 1]
  (2..n).each{|i| ary << (2 * i - 1) * (i * i - i + 1) * ary[-1] - (i - 1) ** 6 * ary[-2]}
  ary
end

n = 200
ary = A(n)
(1..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
