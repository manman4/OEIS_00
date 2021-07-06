def A(k, m, n)
  s = 0
  (1..n).each{|i|
    s += i if n % i == 0 && i % k == m
  }
  s
end

n = 10000
(1..n).each{|i|
  print i
  print ' '
  puts A(4, 1, i)
}
