def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end
def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end
def A(n)
  ary = [0]
  (1..n).each{|i| ary << f(i - 1) - (1..i - 1).inject(0){|s, j| s + ncr(i - 1, j) * f(j - 1) * ary[-j]}}
  ary
end

n = 500
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
