def G(a, n)
  ary = []
  (1..n).each{|j|
    s = 0
    (1..Math.sqrt(j).to_i).each{|k|
      m = Math.sqrt(j - k * k).to_i
      if m * m == j - k * k
        x = (k + m * 1i) ** a
        s += x
      end
    }
    ary << s.real
  }
  ary
end

n = 10000
ary = G(8, n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
