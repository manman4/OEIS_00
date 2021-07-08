require 'prime'

def A008683(n)
  ary = n.prime_division
  return (-1) ** (ary.size % 2) if ary.all?{|i| i[1] == 1}
  0
end

# ary[0] = 1
def inverse_Euler_transform(ary, n)
  c_ary = [0]
  (1..n).each{|i| c_ary << (1..i - 1).inject(i * ary[i]){|s, j| s - ary[j] * c_ary[-j]}}
  m_ary = [0] + (1..n).map{|i| A008683(i)}
  a = [0]
  (1..n).each{|i|
    s = 0
    (1..i).each{|j|
      s += m_ary[i / j] * c_ary[j] if i % j == 0
    }
    if s % i == 0
      a << s / i
    else
      a << s / i.to_r
    end
  }
  a
end

def s(f_ary, g_ary, n)
  s = 0
  (1..n).each{|i| s += i * f_ary[i] * g_ary[i] ** (n / i) if n % i == 0}
  s
end

def A(f_ary, g_ary, n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(f_ary, g_ary, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s + a[j] * ary[-j]} / i}
  ary
end

def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

n = 23
ary0 = (0..n).map{|i| (-4) ** i}
ary = inverse_Euler_transform(ary0, n)
(1..n).each{|i|
  j = -ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
