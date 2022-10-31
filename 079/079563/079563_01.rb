def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(k, n)
  (0..n).inject(0){|s, i| s + (k - 1) ** (n - i) * ncr(k * n + 1, i)}
end

n = 1000
(0..n).each{|i|
  j = A(7, i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}