# a(n) = Sum_{d|n} d*(d-1)/2.
def A(n)
  s = 0
  (1..n).each{|i|
    s += i * (i - 1) / 2 if n % i == 0
  }
  s
end

n = 10000
(1..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
