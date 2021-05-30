def phi(n)
  (1..n).count{|i| i.gcd(n) == 1}
end

def A(k, n)
  ary = []
  i = 1
  while ary.size < n
    ary << i if phi(i + k) == phi(i) + k
    i += 1
  end
  ary
end

n = 1000
ary = A(4, n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
