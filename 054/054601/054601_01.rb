def A(n)
  s = 0
  (1..n).each{|i|
    s += i * 2 ** (n / i - 1) if n % i == 0 && i % 2 == 1
  }
  s
end

n = 1000
(0..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
