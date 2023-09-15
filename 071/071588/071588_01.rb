n = 1000
(0..n).each{|i| 
  j = (6 ** i).to_s.reverse.to_i
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
