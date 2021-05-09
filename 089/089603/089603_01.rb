def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n)
  (0..n).inject(0){|s, i| s += ncr(n, i) * ncr(2 * i, i) * ncr(2 * n - 2 * i, n - i)}
end

# ary[0] = 1
def sqrt_a(ary)
  n = ary.size - 1
  a = [1]
  (0..n - 1).each{|i| a << (ary[i + 1] - (1..i).inject(0){|s, j| s + a[j] * a[-j]}) / 2}
  a
end

n = 500
ary = (0..n).map{|i| A(i)}
ary = sqrt_a(ary)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
