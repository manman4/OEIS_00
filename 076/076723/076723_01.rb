def A(n)
  s = 0
  (1..n).each{|i|
    s += (-i) ** i if n % i == 0
  }
  s
end

n = 50
(1..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
