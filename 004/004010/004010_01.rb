# m���ȉ������o��
def mul(f_ary, b_ary, m)
  s1, s2 = f_ary.size, b_ary.size
  ary = Array.new(s1 + s2 - 1, 0)
  (0..s1 - 1).each{|i|
    (0..s2 - 1).each{|j|
      ary[i + j] += f_ary[i] * b_ary[j]
    }
  }
  ary[0..m]
end

# m���ȉ������o��
def power(ary, n, m)
  return [1] if n == 0
  k = power(ary, n >> 1, m)
  k = mul(k, k, m)
  return k if n & 1 == 0
  return mul(k, ary, m)
end

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

m = 28
n = m + 100
a = [0] + A([[9, 3], [3, -1]], n)
b = A([[1, 3], [3, -1]], n)
c = (0..n).map{|i| 9 * a[i] + b[i]}
d = power(b, 3, n)
e = power(c, 3, n)
f = mul(d, e, n)
g = power(c, 6, n)
h = power(b, 6, n)
(0..m).each{|i|
  j = (3 * g[i] - 4 * f[i] + 4 * h[i]) / 3
  print i
  print ' '
  puts j
}
