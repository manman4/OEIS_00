def A(n)
  (1..n).inject(0){|s, i| s + i ** (n - i) + (-1) ** (n - i) * (n - i) ** i}
end

n = 600
(1..n).each{|i|
  j = A(i)
  print i
  print ' '
  puts j
}
