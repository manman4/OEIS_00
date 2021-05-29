def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A074635(n)
  [1] + (1..n).map{|i| (0..i).inject(0){|s, j| s + ncr(i, j) ** 2 * ncr(i + j, j + 1) ** 2} / (i * i)}
end

n = 700
ary = A074635(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
