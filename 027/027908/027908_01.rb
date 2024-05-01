def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n, s, t, u)
  (0..n / s).inject(0){|sum, k| sum + ncr(t * n, k) * ncr((t + u) * n - k, n - s * k)}
end

n = 1000
(0..n).each{|i|
  j = A(i, 2, 2, 0)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
