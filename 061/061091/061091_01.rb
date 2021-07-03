def phi(n)
  (1..n).count{|i| i.gcd(n) == 1}
end

n = 10000
s = 0
(1..n).each{|i|
  s += 1 if i.gcd(phi(i)) == 1
  print i
  print ' '
  puts s
}
