def A(k, n)
  a = Array.new(n + 1 + k, 1)
  return a if k == 0
  b = (1..n + 1 + k - 1).to_a
  return b if k == 1
  (k - 1).times{|i|
    c = (0..n + k - 2 - i).map{|j| (b[j] * b[j + 1] - 1) / a[j + 1]}
    a, b = b, c
  }
  b
end

n = 10000
ary = A(5, n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
