def f(n)
  (0..n - 1).count{|i| (i * i + 1) % n == 0}
  
end

n = 10000
(1..n).each{|i|
  print i
  print ' '
  puts f(i)
}
