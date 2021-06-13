def A006769(n)
  a = [1, 1, -1, 1]
  ary = [0, 1]
  while ary.size < n + 1
    i = a[1] * a[-1] + a[2] * a[2]
    break if i % a[0] > 0
    a = *a[1..-1], i / a[0]
    ary << a[0]
  end
  ary[0..n]
end

n = 400
ary = A006769(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
