def A019461(n)
  ary = [0]
  i = 0
  s = 0
  while i < n
    i += 1
    if i % 2 > 0
      s += (i + 1) / 2
    else
      s *= (i + 1) / 2
   end
   ary << s
  end
  ary
end

n = 1000
ary = A019461(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
