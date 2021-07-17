def A(n)
  ((n + 1) * (3 + Math.sqrt(5)) / 2).to_i - 1
end

n = 10000
(0..n).each{|i|
  print i
  print ' '
  puts 13 * A(i) - 5 * i
}
