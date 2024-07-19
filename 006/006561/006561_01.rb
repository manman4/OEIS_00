def d(n, k)
  n % k == 0 ? 1 : 0
end

def d2(n)
  d(n, 2)
end

def d4(n)
  d(n, 4)
end

def d6(n)
  d(n, 6)
end

def d12(n)
  d(n, 12)
end

def d18(n)
  d(n, 18)
end

def d24(n)
  d(n, 24)
end

def d30(n)
  d(n, 30)
end

def d42(n)
  d(n, 42)
end

def d60(n)
  d(n, 60)
end

def d84(n)
  d(n, 84)
end

def d90(n)
  d(n, 90)
end

def d120(n)
  d(n, 120)
end

def d210(n)
  d(n, 210)
end

# n > 4
def a2(n)
  s = 0
  s += (n ** 3 - 6 * n * n + 11 * n - 6) / 24r
  s += (- 5 * n * n + 46 * n - 72) / 16r * d2(n)
  s -= 9 / 4r * d4(n)
  s += (-19 * n + 110) / 2r * d6(n)
  s +=  54 * d12(n)
  s +=  84 * d18(n)
  s +=  50 * d24(n)
  s -=  24 * d30(n)
  s -= 100 * d42(n)
  s -= 432 * d60(n)
  s -= 204 * d84(n)
  s -= 144 * d90(n)
  s -= 204 * d120(n)
  s -= 144 * d210(n)
  (n * s).to_i
end

def a3(n)
  s = 0
  s += (5 * n * n - 48 * n + 76) / 48r * d2(n)
  s += 3 / 4r * d4(n)
  s += (7 * n - 38) / 6r * d6(n)
  s -=   8 * d12(n)
  s -=  20 * d18(n)
  s -=  16 * d24(n)
  s -=  19 * d30(n)
  s +=   8 * d42(n)
  s +=  68 * d60(n)
  s +=  60 * d84(n)
  s +=  48 * d90(n)
  s +=  60 * d120(n)
  s +=  48 * d210(n)
  (n * s).to_i
end

def a4(n)
  s = 0
  s += (7 * n - 42) / 12r * d6(n)
  s -= 5 / 2r * d12(n)
  s -=  4 * d18(n)
  s +=  3 * d24(n)
  s +=  6 * d42(n)
  s += 34 * d60(n)
  s -=  6 * d84(n)
  s -=  6 * d120(n)
  (n * s).to_i
end

def a5(n)
  s = 0
  s += (n - 6) / 4r * d6(n)
  s -= 3 / 2r * d12(n)
  s -= 2 * d24(n)
  s += 4 * d42(n)
  s += 6 * d84(n)
  s += 6 * d120(n)
  (n * s).to_i
end

def a6(n)
  n * (4 * d30(n) - 4 * d60(n))
end

def a7(n)
  n * (d30(n) + 4 * d60(n))
end

def a(n, k)
  return a2(n) if k == 2
  return a3(n) if k == 3
  return a4(n) if k == 4
  return a5(n) if k == 5
  return a6(n) if k == 6
  return a7(n) if k == 7
end

def A(n)
  return [0] if n < 4
  return [0, a2(n)] if n % 2 == 1
  return [0] + (2..n / 2 - 1).map{|i| a(n, i)} + [1] if n / 2 < 9
  [0] + (2..7).map{|i| a(n, i)} + [0] * (n / 2 - 8) + [1]
end

# def A292105(n)
#   (1..n).map{|i| A(i)}.flatten
# end

n = 20000

(1..n).each{|i|
  print i
  print ' '
  puts A(i).inject(:+)
}

