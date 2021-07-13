require 'prime'

def A008683(n)
  ary = n.prime_division
  return (-1) ** (ary.size % 2) if ary.all?{|i| i[1] == 1}
  0
end

# ary[0] = 1
def inverse_Euler_transform(ary, n)
  c = [0]
  (1..n).each{|i| c << (1..i - 1).inject(i * ary[i]){|s, j| s - ary[j] * c[-j]}}
  m_ary = [0] + (1..n).map{|i| A008683(i)}
  a = [0]
  (1..n).each{|i|
    s = 0
    (1..i).each{|j|
      s += m_ary[i / j] * c[j] if i % j == 0
    }
    # ary�̗v�f�������Ȃ�As / i�͐���
    a << s / i
  }
  a
end

def A(n)
  ary = [1]
  cnt = 0
  while cnt < n
    cnt += 1
    if cnt % 2 == 0
      ary << ary[cnt / 2]
    else
      ary << 3 - ary[cnt / 2]
    end
  end
  ary
end

n = 50
ary = inverse_Euler_transform([1] + A(n), n)
(1..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
