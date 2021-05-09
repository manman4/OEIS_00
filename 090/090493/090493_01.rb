def A(n)
  return 0 if n == 1 or n == 10 or n == 100 or n == 1000 or n == 10000
  s = 1
  t = n
  while t.to_s.split('').uniq.size < 10
    s += 1
    t *= n
  end
  s
end

n = 10000
(1..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}