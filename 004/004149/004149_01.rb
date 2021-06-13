def A004149(n)
  i = 5
  a = [1, 1, 1, 1, 2, 4]
  ary = [1]
  while ary.size < n + 1
    i += 1
    a = *a[1..-1], ((2 * i + 1) * a[-1] + (i - 1) * a[-2] + (i - 4) * a[-4] - (2 * i - 11) * a[-5] - (i - 7) * a[-6]) / (i + 2)
    ary << a[0]
  end
  ary
end

n = 3000
ary = A004149(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
