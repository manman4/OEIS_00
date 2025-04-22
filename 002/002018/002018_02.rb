def A(n)
  a = [1, 1]
  (2..n).each{|i|
    a[i] = i * (i * a[i - 1] - (i - 1) * (i - 2) / 2 * a[i - 2])
  }
  a
end

n = 500
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}