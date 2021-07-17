require 'prime'

def mu(n)
  return 1 if n == 1
  a = n.prime_division.map{|i| i[1]}
  return 0 if a.max > 1
  (-1) ** (a.size % 2)
end

def A(k, n)
  ary = [1]
  a = [0] + (1..n).map{|i| mu(i)}
  (1..n).each{|i|
    s = 0
    (1..i).each{|j| s += a[i / j] * k ** j if i % j == 0}
    ary << s / i
  }
  ary
end

n = 21
ary = A(5, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
