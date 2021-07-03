def A(n)
  return [0] if n == 0
  ary = [0, 1]
  (2..n).each{|i| ary << ((i - 1) ** 5 + i ** 5) * ary[-1] - (i - 1) ** 10 * ary[-2]}
  ary
end

n = 20
p ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
