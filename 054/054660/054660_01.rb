require 'prime'

def A008683(n)
  ary = n.prime_division
  return (-1) ** (ary.size % 2) if ary.all?{|i| i[1] == 1}
  0
end

def A(n)
  return 1 if n == 0
  s = 0
  (1..n).each{|i|
    s += A008683(i) * 4 ** (n / i) if n % i == 0 && i % 2 == 1
  }
  s / (4 * n)
end

n = 1000
(1..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
