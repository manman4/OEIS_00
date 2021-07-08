(0..10000).each{|i|
  print i
  print ' '
  puts i.to_s(3).split('').sort.join.to_i(3)
}
