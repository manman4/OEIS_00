def s(k, n)
  s = 0
  (1..n).each{|i| s += (-1) ** (n / i + 1) * i ** k if n % i == 0}
  s
end

n = 10000
(1..n).each{|i|
  print i
  print ' '
  puts s(2, i)
}
