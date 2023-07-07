def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A186690(n)
  s = 0
  (1..n).each{|i|
    if n % i == 0
      s += i if (n / i) % 2 == 1
    end
  }
  (-1) ** (n + 1) * s
end

def B(n)
  p ary = [0] + (1..n).map{|i| A186690(i)}
  # a(n) = -(n-1)! * Sum_{k=1..n} ary[k] * a(n-k)/(n-k)!
  a = [1]
  (1..n).each{|i|
    m = f(i - 1)
    a << -(1..i).inject(0){|s, j| s + ary[j] * a[-j] * m / f(i - j)}
  }
  a
end

n = 20
p ary = B(n)
# (0..n).each{|i|
#   j = ary[i]
#   break if j.to_s.size > 1000
#   print i
#   print ' '
#   puts j
# }