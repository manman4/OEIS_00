puts '1 0'
(2..1000).each{|i|
  j = 1
  (1..i - 1).each{|k|
    j *= k
    j %= i
  }
  print i
  print ' '
  puts j
}
