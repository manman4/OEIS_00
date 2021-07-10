N = 200
n = 100
ary = Array.new(N + 1, 0)
(0..n).each{|i|
  (2 * i..n).each{|j|
    k = (3 * (2 * j + 1) * (2 * j + 1) - (6 * i + 1) * (6 * i + 1)) / 24
    ary[k] += (-1) ** (i + j) if k <= N
    if i != 0
      k = (3 * (2 * j + 1) * (2 * j + 1) - (-6 * i + 1) * (-6 * i + 1)) / 24
      ary[k] += (-1) ** (-i + j) if k <= N
    end
  }
}

(0..N).each{|i|
  print i
  print ' '
  puts ary[i]
}
