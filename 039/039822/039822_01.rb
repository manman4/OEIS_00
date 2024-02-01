def A(n)
  a = [1]
  ary = [1]
  (1..n).each{|i|
    b = a + [0] * i
    (0..a.size - 1).each{|j| b[i + j] += a[j]}
    a = b
    ary << a.uniq.size
  }
  ary
end

n = 1100
ary = A(n)
m = 1000
(0..m).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}

