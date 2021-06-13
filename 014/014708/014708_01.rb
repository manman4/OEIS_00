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

def A001158(n)
  s = 0
  (1..n).each{|i| s += i * i * i if n % i == 0}
  s
end

def A004009(n)
  a = [1] + (1..n).map{|i| 240 * A001158(i)}
end

def E4_3(n)
  ary = A004009(n)
  power(ary, 3, n)
end

def s(n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0}
  s
end

def A(k, n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - k * a[j] * ary[-j]} / i}
  ary
end

def f(n)
  a = E4_3(n)
  b = A(-24, n)
  mul(a, b, n)
end

n = 14
ary = f(n + 1)
ary[1] = 0
-1.upto(n){|i|
  j = ary[i + 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
