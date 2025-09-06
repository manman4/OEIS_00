def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n) = sum{k=0..floor(n/2)} 2^(n-2*k) * binomial(2*n-2*k+1, 2*k).
def A(n)
  (0..n / 2).inject(0){|s, i| s + 2 ** (n - 2 * i) * ncr(2 * n - 2 * i + 1, 2 * i)}
end

n = 1000
(0..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
