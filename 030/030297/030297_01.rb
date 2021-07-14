def A030297(n)
  ary = [0]
  i = 0
  s = 0
  while i < n
    i += 1
    s += i
    s *= i
   ary << s
  end
  ary
end

n = 500
ary = A030297(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
