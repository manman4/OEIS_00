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

def I(ary, n)
  a = [1]
  i = 0
  while i < n
    a << -(0..i).inject(0){|s, j| s + ary[1 + i - j] * a[j]}
    i += 1
  end
  a
end

def A(k, n)
  ary1 = [1, 1]
  (2..k).each{|i|
    a = [1] + [0] * (i - 1) + [1]
    ary1 = mul(ary1, a, n)
  }
  ary2 = [1, -1]
  (2..k).each{|i|
    a = [1] + [0] * (i - 1) + [-1]
    ary2 = mul(ary2, a, n)
  }
  ary2 += [0] * (n + 1 - ary2.size)
  mul(ary1, I(ary2, n), n)
end

n = 40
ary = A(7, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
