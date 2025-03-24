def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n, k)
  (1..n / 2).inject(1){|s, i| s + (ncr(n, i) - ncr(n, i - 1)) ** k} / ncr(n, n / 2).to_r
end

n = 1000
k = 3
(0..n).each{|i|
  j = A(i, k)
  break if j.denominator > 1
  j =  j.to_i
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}