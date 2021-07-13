require 'prime'

def pow(a, m, mod)
  return 1 if m == 0
  k = pow(a, m >> 1, mod)
  k *= k
  return k % mod if m & 1 == 0
  return k * a % mod
end

def A015951(n)
  ary = []
  i = 1
  while ary.size < n
    j = pow(5, i, i)
    ary << i if j == (i - 1) % i
    i += 1
  end
  ary
end

n = 500
ary = A015951(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
