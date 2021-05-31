require 'prime'

def pow(a, m, mod)
  return 1 if m == 0
  k = pow(a, m >> 1, mod)
  k *= k
  return k % mod if m & 1 == 0
  return k * a % mod
end

def A067947(n)
  ary = []
  i = 1
  while ary.size < n
    j = pow(7, i, i)
    ary << i if j == 1 % i
    i += 1
  end
  ary
end

n = 10000
ary = A067947(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
