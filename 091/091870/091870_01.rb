def A(k, n)
  ary = [[0]]
  a = [0]
  l_ary = [0]
  (1..n).each{|i|
    m = 0
    m += k ** (i / 2) if i % 2 == 1
    b = [m]
    (1..i).each{|j| b[j] = (k - 1) * a[j - 1] + b[j - 1]}
    a = b
    ary << a
    l_ary << a[-1]
  }
  l_ary
end

n = 30
ary = A(5, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
