(0..10000).each{|i| 
  print i
  print ' '
  puts i - i.to_s(2).split('').sort.join.to_i(2)
}
