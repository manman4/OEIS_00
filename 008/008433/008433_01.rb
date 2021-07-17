def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == 0}
  s
end

def A(ary, n)
  a_ary = [1]
  a = [0] + (1..n).map{|i| ary.inject(0){|s, j| s + j[1] * s(j[0], i)}}
  (1..n).each{|i| a_ary << (1..i).inject(0){|s, j| s - a[j] * a_ary[-j]} / i}
  a_ary
end

def B(k, n)
  a = A([[2, 5 * k], [1, -2 * k], [4, -2 * k]], 2 * n)
  (0..n).map{|i| a[2 * i]}
end

def C(k, n)
  m = 2 ** (k - 1)
  A([[2, 2 * k], [1, -k]], n).map{|i| m * i}
end

def D(k, n)
  a1 = B(k, n / 8)
  a2 = C(k, n / 8)
  ary = Array.new(n + 1, 0)
  (0..n / 8).each{|i| ary[8 * i] = a1[i]}
  k.step(n, 8){|i| ary[i] += a2[(i - k) / 8]}
  ary
end

def E(k, n)
  m = k.gcd(8)
  a = D(k, m * n)
  (0..n).map{|i| a[m * i]}
end

n = 64
ary = E(5, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
