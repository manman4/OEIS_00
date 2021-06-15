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

def g(k, n)
  return 0 if n == 0
  (f(n, k + 1) - 2 * f(n, k) + f(n, k - 1)) / 2
end

n = 1000
(0..n).each{|i|
  print i
  print ' '
  puts g(3, i)
}
