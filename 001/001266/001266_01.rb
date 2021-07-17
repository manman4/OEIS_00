def A001266(n)
  ary = [0, 0, 1, 7]
  i = 5
  while i < n
    i += 1
    ary << (i + 1) * ary[-1] - (i - 2) * ary[-2] - (i - 5) * ary[-3] + (i - 3) * ary[-4]
  end
  ary[0..n - 2]
end

n = 500
ary = A001266(n)
(2..n).each{|i|
  j = ary[i - 2]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
