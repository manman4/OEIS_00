# A(n,k) = Product_{j=1..n} (1 - (-k)^j)/(1 + k).
def A(n, k)
  return 1 if n == 0
  (1..n).inject(1){|s, i| s * (1 - (-k)**i) / (1 + k)}
end

n = 50
(0..n).each{|i| 
  j = A(i, 5)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}