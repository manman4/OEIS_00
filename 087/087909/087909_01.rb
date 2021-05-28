def s(k, n)
  s = 0
  (1..n).each{|i| s += i ** (k * n / i - 1) if n % i == 0}
  s
end

n = 10000
(1..n).each{|i|
  j = s(1, i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
