def s(k, n)
  s = 0
  (1..n).each{|i| s += n / i if n % i == 0 && i % k > 0}
  s
end

n = 10000
(1..n).each{|i|
  print i
  print ' '
  puts s(3, i)
}
