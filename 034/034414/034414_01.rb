def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

n = 1000
print 0
print ' '
puts 1
(1..n).each{|i|
  j = ncr(24 * i, 5) * ncr(5 * i - 2, i - 1) / ncr(4 * i + 4, 5)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
