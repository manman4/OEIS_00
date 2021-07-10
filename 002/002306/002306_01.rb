def c(n, r)
  return 1 if r == 0
  return c(n, n - r) if r > n - r
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def h(n)
  a = [1r / 10]
  (2..n).each{|i| a << 3 * (1..i - 1).inject(0){|s, j| s + (4 * j - 1) * (4 * i - 4 * j - 1) * c(4 * i, 4 * j) * a[j - 1] * a[i - j - 1]} / ((2 * i - 3) * (4 * i - 1) * (4 * i + 1))}
  a
end

n = 13
ary = h(n).map{|i| i.numerator}
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
