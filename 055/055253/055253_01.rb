s = 1
puts '0 0'
(1..10 ** 4).each{|i|
  s *= 2
  print i
  print ' '
  puts s.to_s.split('').map(&:to_i).select{|i| i % 2 == 0}.size
}
