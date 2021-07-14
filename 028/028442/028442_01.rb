require 'prime'

def A(n)
  return 1 if n == 1
  a = n.prime_division.map{|i| i[1]}
  return 0 if a.max > 1
  (-1) ** (a.size % 2)
end

def B(k, n)
  s = 0
  ary = []
  i = 0
  while ary.size < n
    i += 1
    s += A(i)
    ary << i if s == k
  end
  ary
end

n = 10000
ary = B(0, n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
