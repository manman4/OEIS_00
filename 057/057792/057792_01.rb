def A(n)
  s = 0
  (1..n).each{|i|
    s += i ** i if i.gcd(n) == 1
  }
  s
end

n = 1000
(1..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}