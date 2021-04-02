def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n)
  ncr(n+5,n+4) * ncr(n+6, n+3) * ncr(n+7, n+2) * ncr(n+8, n+1) * ncr(n+9, n) / (140 * 120)
end

n = 10000
(0..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
