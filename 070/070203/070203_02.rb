def A(n)
  cnt = 0
  (1..n / 3).each{|a|
    (a + 1..(n - a) / 2).each{|b|
      c = n - a - b
      if a + b > c && b < c
        s = n / 2r
        t = (s - a) * (s - b) * (s - c) / s
        if t.denominator == 1
          t = t.to_i
          cnt += 1 if Math.sqrt(t).to_i ** 2 == t
        end
      end
    }
  }
  cnt
end
def A070203(n)
  (1..n).map{|i| A(i)}
end
p A070203(100) 

# n = 50
# (1..n).each{|i|
#   print i
#   print ' '
#   puts A(i)
# }
