def A(n)
  a = [0, 0, 0, 0]
  b = [1]
  ary = []
  (n + 1).times{|i|
    ary << b
    b = ([0, 0] + b + [0, 0])
    a, b = b, (0..i + 1).map{|j| a[j] + a[j + 1] + a[j + 2] + b[j + 1] + b[j + 2]}
  }
  ary.flatten
end

n = 139
ary = A(n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
