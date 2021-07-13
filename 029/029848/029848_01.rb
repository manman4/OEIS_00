def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

n = 100
b=[]
(0..n).each{|i|
  j = 1 + ncr(2 * i, i) + ncr(3 * i, i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
b<<j
}
p b
