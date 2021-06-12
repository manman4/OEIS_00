(0..8191).each{|i| 
  print i
  print ' '
  puts i.to_s(2).split('').sort.reverse.join.to_i(2)
}
