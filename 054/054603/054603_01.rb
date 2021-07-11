def phi(n)
  (1..n).count{|i| i.gcd(n) == 1}
end

def A(k, n)
  s = 0
  (1..k).each{|i|
    s += phi(i) * n ** (k / i) if k % i == 0
  }
  s
end

n = 10000
(0..n).each{|i|
  j = A(4, i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
