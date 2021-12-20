def f(n)
  (0..(n - 1) / 2).inject(0){|s, i| s + (-1) ** (i % 2) * (n / (2 * i + 1))}
end

n = 1000
(0..n).each{|i|
  print i
  print ' '
  puts f(i * i)
}
