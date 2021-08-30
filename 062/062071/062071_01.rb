def A(n)
  s = 0
  (1..n).each{|i|
    j = i ** i
    break if j > n
    s += (n / j)
  }
  s
end

n = 10000
(1..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
