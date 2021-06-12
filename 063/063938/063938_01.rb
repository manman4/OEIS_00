require 'prime'

# m次以下を取り出す
def mul(f_ary, b_ary, m)
  s1, s2 = f_ary.size, b_ary.size
  ary = Array.new(s1 + s2 - 1, 0)
  s10 = [s1 - 1, m].min
  (0..s10).each{|i|
    s20 = [s2 - 1, m - i].min
    (0..s20).each{|j|
      ary[i + j] += f_ary[i] * b_ary[j]
    }
  }
  ary
end

# m次以下を取り出す
def power(ary, n, m)
  return [1] if n == 0
  k = power(ary, n >> 1, m)
  k = mul(k, k, m)
  return k if n & 1 == 0
  return mul(k, ary, m)
end

def A000594(n)
  ary = Array.new(n + 1, 0)
  # ヤコビの公式の必要なところだけ取り出す
  i = 0
  j, k = 2 * i + 1, i * (i + 1) / 2
  while k <= n
    i & 1 == 1? ary[k] = -j : ary[k] = j
    i += 1
    j, k = 2 * i + 1, i * (i + 1) / 2
  end
  # 8乗してx倍
  power(ary, 8, n).unshift(0)[1..n]
end

n = 49500
ary = A000594(n)
cnt = 0
(1..n).each{|i|
  if ary[i - 1] % i == 0
    cnt += 1
    print cnt
    print ' '
    puts i
  end
}
