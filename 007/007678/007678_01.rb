# nがmで割り切れるとき1,そうでないときは0
def d(n, m)
  n % m == 0 ? 1 : 0
end

def A(n)
  s = (n - 1) * (n - 2) * (n * n - 3 * n + 12) / 24r
  s += (- 5* n ** 3 + 42 * n * n - 40 * n - 48) / 48r * d(n, 2)
  s -= 3 * n / 4r * d(n, 4)
  s += (-53 * n * n + 310 * n) / 12r * d(n, 6)
  s += 49 * n / 2r * d(n, 12)
  s +=  32 * n * d(n, 18)
  s +=  19 * n * d(n, 24)
  s -=  36 * n * d(n, 30)
  s -=  50 * n * d(n, 42)
  s -= 190 * n * d(n, 60)
  s -=  78 * n * d(n, 84)
  s -=  48 * n * d(n, 90)
  s -=  78 * n * d(n, 120)
  s -=  48 * n * d(n, 210)
  s.to_i
end

n = 10000
(1..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}