def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n) = 2^(3*n-1) - 3 * Sum_{k=0..n-2} binomial(3*n-1,k) for n > 0.
def a(n)
  return 1 if n == 0
  2 ** (3 * n - 1) - 3 * (0..n - 2).inject(0){|s, k| s + ncr(3 * n - 1, k)}
end

n = 1000
(0..n).each{|i|
  j = a(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
