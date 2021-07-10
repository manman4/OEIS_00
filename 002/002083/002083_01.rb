def A002083(n)
  return [1] if n == 1
  ary = [1, 1]
  (3..n).each{|i|
    j = 2 * ary[-1]
    j -= ary[(i - 1) / 2 - 1] if i % 2 == 1
    ary << j
  }
  ary
end

n = 4000
ary = A002083(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
