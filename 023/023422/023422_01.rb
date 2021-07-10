def A023422(n)
  ary = [1]
  i = 0
  while i < n
    ary << (4..i - 1).inject(ary[-1]){|s, j| s + ary[j] * ary[i - 1 - j]}
    i += 1
  end
  ary
end

n = 30
ary = A023422(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
