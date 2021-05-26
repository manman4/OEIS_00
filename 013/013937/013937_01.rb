def A(n)
  (1..n).inject(0){|s, i| s + (n / i ** 3)}
end

n = 10000
(0..n).each{|i|
  j = A(i)
  print i
  print ' '
  puts j
}
