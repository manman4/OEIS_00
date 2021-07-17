def A001515(n)
  ary = [1, 2]
  i = 1
  while i < n
    i += 1
    ary << (2 * i - 1) * ary[-1] + ary[-2]
  end
  ary[0..n]
end

n = 500
ary = A001515(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
