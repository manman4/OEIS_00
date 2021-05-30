def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n)
  return 0 if n == 0
  (0..n - 1).inject(0){|s, i| s + 1r / ncr(n - 1, i)} / n
end

n = 1000
(0..n).each{|i|
  j = A(i).numerator
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
