def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# f(x+1) - f(x) = (aryが表す多項式) を満たすf(x)を求める
def A(ary)
  m = ary.size
  a = []
  m.downto(1){|i|
    b = ary[0] / i.to_r
    a << b
    (1..i).each{|j| ary[j - 1] -= b * ncr(i, j)}
    ary = ary[1..-1]
  }
  a
end

# aryはm-1次
def B(ary)
  m = ary.size
  a = Array.new(m, 0)
  (0..m - 1).each{|i|
    b = ary[i]
    (i..m - 1).each{|j|
      a[j] += b * (-1) ** (j - i) * ncr(m - 1 - i, j - i)
    }
  }
  a
end

def A005773(n)
  ary = [1, 1]
  (2..n).each{|i|
    ary << (2 * i * ary[-1] + 3 * (i - 2) * ary[-2]) / i
  }
  ary
end

def A098494(n)
  c_ary = A005773(n)
  ary = [[1], [1, -1]]
  (2..n).each{|i|
    a, b = ary[-1], ary[-2]
    c = [0] + B(b).map{|j| (i - 1) * j}
    ary << A((0..i - 1).map{|j| i * (a[j] - c[j])}) + [(-1) ** i * f(i) * c_ary[i]]
  }
  ary.flatten
end

n = 139
ary = A098494(n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.denominator > 1
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j.to_i
}