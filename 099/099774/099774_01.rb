def A(n)
  s = 0
  (1..n).each{|i|
    s += 1 if n % i == 0
  }
  s
end

def B(n)
  (1..n).map{|i| A(2 * i - 1)}
end

n = 105
p ary = B(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
