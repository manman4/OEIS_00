def d(n, k)
  n % k == 0 ? 1 : 0
end

def a2(n)
  s  = (n ** 3 - 6 * n * n + 11 * n - 6) / 24r
  s += (- 5 * n * n + 46 * n - 72) / 16r * d(n, 2)
  s -= 9 / 4r * d(n, 4)
  s += (-19 * n + 110) / 2r * d(n, 6)
  s +=  54 * d(n, 12)
  s +=  84 * d(n, 18)
  s +=  50 * d(n, 24)
  s -=  24 * d(n, 30)
  s -= 100 * d(n, 42)
  s -= 432 * d(n, 60)
  s -= 204 * d(n, 84)
  s -= 144 * d(n, 90)
  s -= 204 * d(n, 120)
  s -= 144 * d(n, 210)
  (n * s).to_i
end

def a3(n)
  s  = (5 * n * n - 48 * n + 76) / 48r * d(n, 2)
  s += 3 / 4r * d(n, 4)
  s += (7 * n - 38) / 6r * d(n, 6)
  s -=   8 * d(n, 12)
  s -=  20 * d(n, 18)
  s -=  16 * d(n, 24)
  s -=  19 * d(n, 30)
  s +=   8 * d(n, 42)
  s +=  68 * d(n, 60)
  s +=  60 * d(n, 84)
  s +=  48 * d(n, 90)
  s +=  60 * d(n, 120)
  s +=  48 * d(n, 210)
  (n * s).to_i
end

def a4(n)
  s  = (7 * n - 42) / 12r * d(n, 6)
  s -= 5 / 2r * d(n, 12)
  s -=  4 * d(n, 18)
  s +=  3 * d(n, 24)
  s +=  6 * d(n, 42)
  s += 34 * d(n, 60)
  s -=  6 * d(n, 84)
  s -=  6 * d(n, 120)
  (n * s).to_i
end

def a5(n)
  s  = (n - 6) / 4r * d(n, 6)
  s -= 3 / 2r * d(n, 12)
  s -= 2 * d(n, 24)
  s += 4 * d(n, 42)
  s += 6 * d(n, 84)
  s += 6 * d(n, 120)
  (n * s).to_i
end

def a6(n)
  n * (4 * d(n, 30) - 4 * d(n, 60))
end

def a7(n)
  n * (d(n, 30) + 4 * d(n, 60))
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

