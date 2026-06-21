def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# T(n, k) = Sum_{j=0..k} (-1)^j*(2*n*(k-j)!/(n+k-j))*binomial(n-k+j, n-k)*binomial(n+k-j, n-k+j), with T(0, k) = 1.
def T(n, k)
  return 1 if n == 0
  (0..k).inject(0){|s, j| s + (-1)**j * (2*n*(f(k-j)/(n+k-j).to_r)) * ncr(n-k+j, n-k) * ncr(n+k-j, n-k+j)}.to_i
end

n = 15
b=[]
(5..n).each{|i|
  j = T(i, i - 5)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
  b<<j
}
p b
