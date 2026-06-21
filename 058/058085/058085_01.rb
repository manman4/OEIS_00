def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n) = -Sum_{k=5..n} (-1)^k * (n-k)! * binomial(k,5) * binomial(2*n-k,k).
def a(n)
  -(5..n).inject(0){|s, k| s + (-1)**k * f(n-k) * ncr(k, 5) * ncr(2*n-k, k)}
end

n = 1000
(5..n).each{|i|
  j = a(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
