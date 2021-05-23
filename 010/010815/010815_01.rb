def f(k, n)
  ary = Array.new(n + 1, 0)
  i = 0
  while true
    j = ((k + 1) * i * i - (k - 1) * i) / 2
    break if j > n
    ary[j] = (-1) ** i
    i += 1
  end
  i = 1
  while true
    j = ((k + 1) * i * i + (k - 1) * i) / 2
    break if j > n
    ary[j] = (-1) ** i
    i += 1
  end
  ary
end

k = 2
n = 10000
ary = f(k, n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
