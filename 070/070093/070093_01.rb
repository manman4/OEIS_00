n = 1000
ary = Array.new(n + 1, 0)
(1..n).each{|i|
  (i..n).each{|j|
    (j..n).each{|k|
      ijk = i + j + k
      break if ijk > n
      if i + j > k
        ary[ijk] += 1 if i * i + j * j > k * k
      end
    }
  }
}

(1..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
