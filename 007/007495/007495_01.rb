def f(n, m)
  return 1 if n == 1
  (m - 1 + f(n - 1, m)) % n + 1
end

def A007495(n)
  (1..n).map{|i| f(i, i)}
end

n = 5000
ary = A007495(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
