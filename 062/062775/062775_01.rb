def A(k, n)
  a = []
  b = Array.new(n, 0)
  (0..n - 1).each{|i|
    j = i ** k % n
    a[i] = j
    b[j] += 1
  }
  s = 0
  (0..n - 1).each{|i|
    (0..n - 1).each{|j|
      s += b[(a[i] + a[j]) % n]
    }
  }
  s
end

n = 5000
(1..n).each{|i|
  print i
  print ' '
  puts A(2, i)
}
