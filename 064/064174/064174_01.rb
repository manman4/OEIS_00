# m次以下を取り出す
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

def f(n)
  ary = [0] * (n + 1)
  # Sum_{k>=1} (-1)^(k-1)*x^(k*(3*k-1)/2)
  s = 1
  while
    t = s * (3 * s - 1) / 2
    break if t > n
    ary[t] += (-1) ** (s - 1)
    s += 1
  end
  ary
end

def s(f_ary, g_ary, n, m)
  s = 0
  (m..n).each{|i| s += i * f_ary[i] * g_ary[i] ** (n / i) if n % i == 0}
  s
end

def A(ary, n, m = 1)
  a_ary = [1]
  a = [0] + (1..n).map{|i| ary.inject(0){|s, j| s + s(j[0], j[1], i, m)}}
  (1..n).each{|i| a_ary << (1..i).inject(0){|s, j| s + a[j] * a_ary[-j]} / i}
  a_ary
end

def g(n)
  ary1 = Array.new(n + 1, 1)
  # Product_{k>0} 1/(1 - x^k)
  A([[ary1, ary1]], n)
end

def h(n)
  mul(f(n), g(n), n)
end

n = 48
m = 48
p ary = h(n)
(1..m).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
