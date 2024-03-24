def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

n = 10000
(1..n).each{|i|
  j = i * i
  print i
  print ' '
  puts ncr(j, i) % j
}