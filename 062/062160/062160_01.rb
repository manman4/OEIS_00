def A(k, n)
  (n ** k - (-1) ** (k % 2)) / (n + 1)
end

def B(k, n)
  (0..n).map{|i| A(k, i)}
end

def A062160(n)
  a = []
  (0..n).each{|i| a << B(i, n - i)}
  ary = []
  (0..n).each{|i|
    (0..i).each{|j|
      ary << a[i - j][j]
    }
  }
  ary
end

n = 139
ary = A062160(n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
