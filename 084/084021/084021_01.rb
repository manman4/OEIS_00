def A084021(n)
  n * (10 ** n.to_s.size - 1 - n)
end
(0..9999).each{|i| 
  print i
  print ' '
  puts A084021(i)
}
