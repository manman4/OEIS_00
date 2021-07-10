def A007754(n)
  a = [1, 1]
  i = 1
  ary = [1, 1, 1]
  while ary.size < n + 1
    b = [1, i + 1]
    ary << 1
    ary << i + 1
    (1..i).each{|i|
      j = (b[i] * a[i] - 1) / a[i - 1]
      b << j
      ary << j
    }
    a = b
    i += 1
  end
  ary[0..n]
end

n = 10000
ary = A007754(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
