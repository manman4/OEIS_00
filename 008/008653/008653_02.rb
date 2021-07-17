def A(k, m, n)
  s = 0
  (1..n).each{|i|
    s += i if n % i == 0 && (i % k == m || i % k == k - m)
  }
  s
end

n = 54
print 0
print ' '
puts 1
(1..n).each{|i|
  print i
  print ' '
  puts 12 * A(3, 1, i)
}
