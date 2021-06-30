def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n)
  (0..n / 2).inject(0){|s, i| s + (-1) ** (i % 2) * ncr(n, i) * (n - 2 * i) ** (n - 1)} / n
end

n = 1000
(1..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}