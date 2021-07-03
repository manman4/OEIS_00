n = 10000
(0..n).each{|i|
  print i
  print ' '
  puts ((7 + Math.sqrt(1 + 48 * i)) / 2).to_i
}
