# a(n) = n!*Sum_{k=1..n} (k-1)^k/k!.
def A076483(n)
  s = 0
  a = 1r
  (1..n).each{|i|
    a *= i
    s += (i - 1) ** i / a
  }
  s * a
end

n = 500
(0..n).each{|i|
  j = A076483(i).to_i
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
