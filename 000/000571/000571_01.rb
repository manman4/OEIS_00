def phi(n)
  (1..n).count{|i| i.gcd(n) == 1}
end

def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n)
  s = 0
  (1..n).each{|i|
    s += (-1) ** ((n + i) % 2) * phi(n / i) * ncr(2 * i, i) if n % i == 0
  }
  s / (2 * n)
end

def A000571(n)
  a_ary = [1]
  a = [0] + (1..n).map{|i| A(i)}
  (1..n).each{|i| a_ary << (1..i).inject(0){|s, j| s + a[j] * a_ary[-j]} / i}
  a_ary
end

n = 32
ary = A000571(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
