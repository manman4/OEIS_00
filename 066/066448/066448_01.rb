def A(n)
  ary = [[1]]
  (1..n).each{|i|
    a = [0] * (i + 1)
    (1..i).each{|j|
      a[j] += ary[i - j][j]     if i - j >= j
      k = i - 2 * j + 1
      a[j] += ary[k][j - 1]     if k     >= 0 && k     >= j - 1
      a[j] += ary[k - j][j - 1] if k - j >= 0 && k - j >= j - 1
    }
    ary << a
  }
  ary
end

n = 139
ary = A(n).flatten
(0..ary.size - 1).each{|i|
  print i
  print ' '
  puts ary[i]
}

