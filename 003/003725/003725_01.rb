# mŸˆÈ‰º‚ğæ‚èo‚·
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
  return 1 if n < 2
  (1..n).inject(:*)
end

def S(c_ary, x_ary, n)
  ary = Array.new(n + 1, 0)
  tq = x_ary
  tqn = [1] + Array.new(n, 0)
  (0..c_ary.size - 1).each{|i|
    (0..n).each{|j|
      ary[j] += c_ary[i] * tqn[j]
    }
    tqn = mul(tqn, tq, n)
  }
  ary
end

def A(k, n)
  c_ary = (0..n).map{|i| 1r / f(i)}
  x_ary = [0, -1]
  x_ary = S(c_ary, x_ary, n)
  x_ary = [0] * k + x_ary
  x_ary = S(c_ary, x_ary, n)
  (0..n).map{|i| (f(i) * x_ary[i]).to_i}
end

n = 200
ary = A(1, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
