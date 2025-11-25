def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n) = (4*binomial(2*n+3,n)+6*binomial(2*n+1,n+3))/(n+4).
def A071723(n)
  (4 * ncr(2 * n + 3, n) + 6 * ncr(2 * n + 1, n + 3)) / (n + 4)
end

n = 1000
(0..n).each{|i|
  j = A071723(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
  