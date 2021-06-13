def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end
def A014307(n)
  a = [1]
  (1..n).each{|i| a << a[-1] + (1..i - 1).inject(0){|s, j| s + ncr(i - 1, j - 1) * a[j]}}
  a
end

n = 21
ary = A014307(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
