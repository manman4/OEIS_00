require 'prime'

def A008683(n)
  ary = n.prime_division
  return (-1) ** (ary.size % 2) if ary.all?{|i| i[1] == 1}
  0
end

# nsk—ñ–Ú
def T(n, k)
  m = n.gcd(k)
  s = 0
  (1..m).each{|i|
    s += i * A008683(n / i) if m % i == 0
  }
  s
end

# ns–Ú
def A(n, k)
  (1..k).map{|i| T(n, (i - 1) % n + 1)}
end

n = 140
a = []
(1..n).each{|i| a << A(i, n + 1 - i)}
ary = []
(0..n - 1).each{|i|
  (0..i).each{|j|
    ary << a[j][i - j]
  }
}
(1..ary.size).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
