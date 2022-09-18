def A(x, y)
  m = y / x
  m += 1 if y % x > 0
  m
end

def B(n, a = [])
  return [1] if n == 1
  x, y = n.numerator, n.denominator
  return a if y == 1
  m = A(x, y)
  a << m
  x, y = (-y) % x, y * m
  n = x.to_r / y
  return B(n, a)
end

def C(n)
  ary = []
  (2..n).each{|i|
    (1..i - 1).each{|j|
      x = j.to_r / i
      a = B(x)
      # 念のため確認
      if x == a.inject(0){|s, k| s + 1r / k}
        ary << a.size
      else
        break
      end
    }
  }
  ary.flatten
end

n = 141
ary = C(n)
(2..ary.size + 1).each{|i|
  j = ary[i - 2]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
