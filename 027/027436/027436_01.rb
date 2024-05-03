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

# [1, f, f^2, f^3, ...]
def A(f_ary, n)
  g_ary = [1] + [0] * n
  a = [g_ary]
  n.times{
    g_ary = mul(f_ary, g_ary, n)
    a << g_ary
  }
  a
end

# f_aryの1次の項は1であること
def B(f_ary, n)
  a = A(f_ary, n)
  b = []
  (0..n).each{|i|
    c = [0] * (n + 1)
    c[i] = 1
    b << c
  }
  (2..n).each{|i|
    # 計算の順に注意
    (i - 1).downto(1){|k|
      b[k][i] = (a[k][i] - (k + 1..i - 1).inject(0){|s, j| s + b[j][i] * b[k][j]}) / 2
    }
  }
  b[1]
end

n = 600
m = 500
f_ary = [0, 1, 4] + [0] * (n - 2)
ary = B(f_ary, n)
(0..m).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
