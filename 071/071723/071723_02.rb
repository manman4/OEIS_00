def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n) = (Sum_{k=0..n} (k+1)*(k^2+k+1)*binomial(2*n-k,n))/(n+1).
def A071723(n)
  (0..n).inject(0){|s, k| s + (k + 1) * (k * k + k + 1) * ncr(2 * n - k, n)} / (n + 1)
end

n = 1000
(0..n).each{|i|
  j = A071723(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
  