def A(n)
  cnt = 0
  (1..n).each{|i| cnt += 1 if n % i == 0}
  cnt
end

n = 10000
puts '0 0'
puts '1 0'
s = 0
(2..n).each{|i|
  s += (A(i) / 2.0).ceil
  print i
  print ' '
  puts s
}

