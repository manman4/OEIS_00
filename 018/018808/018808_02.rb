def f(n, k)
  s = 0
  (-n).upto(n){|x|
    (-n).upto(n){|y|
      if x.gcd(y) == k
        s += (n - x.abs) * (n - y.abs)
      end
    }
  }
  s
end

def g(n)
  return 0 if n == 0
  (f(n, 1) - f(n, 2)) / 2
end

n = 100
(0..n).each{|i|
  print i
  print ' '
  puts g(i)
}
