def A(n)
  s = 0
  (1..n).each{|i| s += 3 ** (i - 1) if n % i == 0}
  s
end

n = 2000
(1..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
