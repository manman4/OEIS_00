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

n = 92
ary = A([[2, 5], [1, -2], [4, -2]], n)
ary2 = Array.new(n + 1, 0)
(0..n / 2).each{|i| ary2[2 * i] = ary[i]}
ary32 = Array.new(n + 1, 0)
(0..n / 32).each{|i| ary32[32 * i] = ary[i]}
ary00 = mul(mul(ary, ary2, n), ary32, n)
(0..n).each{|i|
  print i
  print ' '
  puts ary00[i]
}
