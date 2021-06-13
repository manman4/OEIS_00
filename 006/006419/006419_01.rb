def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A006419(n)
  (0..n).map{|i| 2 ** (2 * i + 1) - ncr(2 * i + 3, i + 1) + ncr(2 * i + 1, i)}
end

n = 1700
ary = A006419(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
