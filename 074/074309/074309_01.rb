def A(n)
  (n + 1..2 * n).inject(0){|s, i| s + i ** i}
end

n = 1000
(1..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}

