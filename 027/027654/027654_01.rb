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

def A001158(n)
  s = 0
  (1..n).each{|i| s += i * i * i if n % i == 0}
  s
end

def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == 0}
  s
end

def A(ary, n)
  a_ary = [1]
  a = [0] + (1..n).map{|i| ary.inject(0){|s, j| s + j[1] * s(j[0], i)}}
  (1..n).each{|i| a_ary << (1..i).inject(0){|s, j| s - a[j] * a_ary[-j]} / i}
  a_ary
end

def A027652(n)
  a = [-1] + Array.new(n, 0)
  (1..n / 4).each{|i| a[i * 4] -= 240 * A001158(i)}
  mul(A([[1, 2], [2, -1], [4, -6]], n), a, n)
end

n = 23
ary = A027652(4 * n + 1)
(0..n).each{|i|
  j = -ary[4 * i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
