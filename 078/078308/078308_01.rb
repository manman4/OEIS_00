def s(k, n)
  s = 0
  (1..n).each{|i| s += i ** (1 + k * n / i) if n % i == 0}
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
