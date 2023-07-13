s = 1
puts '0 1'
(1..10 ** 4).each{|i|
  s *= 3
  print i
  print ' '
  puts s.to_s.split('').map(&:to_i).select{|i| i % 2 == 1}.size
}
