def A055207(n)
  n.to_s.split('').inject(0){|s, i| s + i.to_i ** n}
end

n = 1050
(0..n).each{|i|
  j = A055207(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}