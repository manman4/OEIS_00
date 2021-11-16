def A(n)
  n.to_s.split('').inject(0){|s, i| s + i.to_i}
end

n = 10000
(0..n).each{|i|
  print i
  print ' '
  puts A(i ** 4)
}
