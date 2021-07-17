def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end
def A(k, n)
  ary = [1]
  (1..n).each{|i| ary << (0..[i - 1, k - 1].min).inject(0){|s, j| s + ncr(i - 1, j) * ary[i - 1 - j]}}
  ary
end

n = 25
ary = A(3, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
