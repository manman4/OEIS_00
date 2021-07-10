require 'prime'

def A008683(n)
  ary = n.prime_division
  return (-1) ** (ary.size % 2) if ary.all?{|i| i[1] == 1}
  0
end

def T(n, k)
  m = n.gcd(k)
  s = 0
  (1..m).each{|i|
    s += i * A008683(n / i) if m % i == 0
  }
  s
end

def A054533(n)
  (1..n).map{|i| (1..i).map{|j| T(i, j)}}
end

n = 140
ary = A054533(n).flatten
(1..ary.size).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
