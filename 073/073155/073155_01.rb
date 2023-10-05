def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n) = Sum_{k=0..n} binomial(2k,n-k)*binomial(2*k,k)/(k+1).
def A(n)
  (0..n).inject(0){|s, i| s + ncr(2 * i, n - i) * ncr(2 * i, i) / (i + 1)}
end

def A073155(n)
  (0..n).map{|i| A(i)}
end

n = 1000
ary = A073155(n)
(0..n).each{|i| 
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
