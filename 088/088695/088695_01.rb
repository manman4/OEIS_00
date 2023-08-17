def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n) = n! * Sum_{k=0..n} binomial(k,n-k)*(n+1)^(k-1)/k!.
def A(n)
  return 1 if n == 0
  m = f(n)
  (1..n).inject(0){|s, i| s + ncr(i, n - i) * (n + 1) ** (i - 1) * m / f(i)} 
end

n = 1000
(0..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
