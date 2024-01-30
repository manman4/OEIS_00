def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end
      
# a(n) = Sum_{k = 0..n} binomial(n,k)^3.
def A(n)
  (0..n).inject(0){|s, i| s + ncr(n, i) ** 3}
end

n = 500
(0..n).each{|i|
  j = A(i) ** 2
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}