def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A(n)
  s = 0
  (0..n).each{|i|
    (0..i).each{|j|
      (0..n).each{|k|
        if i + j + k == n
          s += f(n + k) / (f(i) * f(j) * f(2 * k)).to_r
        end
      }
    }
  }
  s
end

n = 100
(0..n).each{|i|
  j = A(i)
  break if j.denominator > 1
  print i
  print ' '
  puts j.to_i
}

