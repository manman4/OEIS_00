def phi(n)
  (1..n).count{|i| i.gcd(n) == 1}
end

n = 10000
s = 0
(1..n).each{|i|
  s += phi(2 * i - 1)
  break if s.to_s.size > 1000
  print i
  print ' '
  puts s
}
