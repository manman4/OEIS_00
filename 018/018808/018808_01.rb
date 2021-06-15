def f(n, k)
  s = 0
  m = n / k
  (-m).upto(m){|x|
    (-m).upto(m){|y|
      if x.gcd(y) == 1
        s += (n - (k * x).abs) * (n - (k * y).abs)
      end
    }
  }
  s
end

def g(n)
  return 0 if n == 0
  (f(n, 1) - f(n, 2)) / 2
end

n = 1000
(0..n).each{|i|
  print i
  print ' '
  puts g(i)
}
