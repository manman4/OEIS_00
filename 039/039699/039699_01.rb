def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n)
  ncr(2 * n, n) * (0..n).inject(0){|s, i| s + ncr(n, i) ** 2 * ncr(2 * n - 2 * i, n - i) * ncr(2 * i, i)}
end

n = 900
(0..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
