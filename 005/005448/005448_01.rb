n = 10000
(1..n).each{|i|
  print i
  print ' '
  puts 3 * i * (i - 1) / 2 + 1
}
