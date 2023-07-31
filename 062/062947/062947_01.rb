def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

n = 5000
(0..n).each{|i|
  j = ncr(i, i / 7)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
