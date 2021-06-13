def f(n)
  (0..(n - 1) / 2).inject(0){|s, i| s + (-1) ** (i % 2) * (n / (2 * i + 1))}
end

def A014200(n)
  (0..n).map{|i| f(i)}
end

n = 58
ary = A014200(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
