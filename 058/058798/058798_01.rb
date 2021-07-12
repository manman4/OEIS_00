def A(k, n)
  a = Array.new(n + 1 + k, 1)
  b = (1..n + 1 + k - 1).to_a
  ary = [1, b[k]]
  return ary[0..n] if n <= 1
  (n - 1).times{|i|
    c = (0..n + k - 2 - i).map{|j| (b[j] * b[j + 1] - 1) / a[j + 1]}
    a, b = b, c
    ary << b[k]
  }
  ary
end

n = 500
ary = [0] + A(1, n - 1)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
