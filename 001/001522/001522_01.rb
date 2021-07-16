def A001522(n)
  ary = [1]
  a = []
  m = Math.sqrt(2 * n).to_i
  (m + 1).times{a << Array.new(n + 1){0}}
  a[0][0] = 1
  (1..n).each{|x|
    b = Marshal.load(Marshal.dump(a))
    (1..m).each{|i| (0..n - x).each{|j| b[i][j + x] += a[i - 1][j]}}
    a = b
    # 1, 1, … , 1の真っ平らなもの
    s = 1
    # 山型
    (1..x - 1).each{|i|
      # k + 1 <= m
      (1..m - 1).each{|k|
        s += a[k][i] * a[k + 1][x - i]
        break if a[k][x] == 0
      }
    }
    ary << s
  }
  ary
end

n = 10000
ary = A001522(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
