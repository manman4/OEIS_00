def phi(n)
  (1..n).count{|i| i.gcd(n) == 1}
end

def A(n)
  s = 0
  (1..2 * n + 1).each{|i|
    s += phi(i) * 2 ** ((2 * n + 1) / i) if (2 * n + 1) % i == 0
  }
  s / ( 4 * n + 2)
end

n = 2000
(0..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
