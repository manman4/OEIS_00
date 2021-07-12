def A(n)
  return 1 if n < 2
  (n..2 * n - 1).inject(:*)
end

n = 500
s = 0
(1..n).each{|i|
  s += 1r / i
  j = (s * A(i)).to_i
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
