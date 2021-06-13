def f(n)
  (0..(n - 1) / 2).inject(0){|s, i| s + (-1) ** (i % 2) * (n / (2 * i + 1))}
end

def A014198(n)
  (0..n).map{|i| 4 * f(i)}
end

n = 59
ary = A014198(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
