def phi(n)
  (1..n).count{|i| i.gcd(n) == 1}
end

def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A003239(n, k)
  return 1 if n == 0
  m = n.gcd(k)
  s = 0
  (1..m).each{|i|
    s += phi(i) * ncr(n / i, k / i) if m % i == 0
  }
  s / n
end

n = 139
ary = []
(0..n).each{|i|
  (0..i).each{|j|
    ary << A003239(i, j)
  }
}
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
