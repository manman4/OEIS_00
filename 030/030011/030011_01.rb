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

n = 55
ary0 = [1, 1] + Prime.take(n).to_a
ary = inverse_Euler_transform(ary0, n)
(1..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
