def A(n)
  s, t = 0, -1
  (0..n).each{|i|
    j = i ** (n + 1 - i)
    s, t = j, i if j > s
  }
  t
end

n = 10000
(1..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}