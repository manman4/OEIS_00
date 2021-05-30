def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n)
  (0..n).inject(0){|s, i| s + ncr(n, i) * (n - 2 * i) ** (n - 2)} / 2 ** n
end

n = 500
(1..n).each{|i|
  j = A(2 * i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
