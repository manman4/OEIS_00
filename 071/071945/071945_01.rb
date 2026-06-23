def triangle(n)
  ary = Array.new(n + 1){|i| Array.new(i + 1, 0)}
  ary[0][0] = 1
  (0..n).each{|i|
    (0..i).each{|j|
      next if i == 0 && j == 0
      a = i > 0 && j <= i - 1 ? ary[i - 1][j] : 0
      b = j > 0 ? ary[i][j - 1] : 0
      c = i > 1 && j > 0 && j <= i - 1 ? ary[i - 2][j - 1] : 0
      ary[i][j] = a + b + c
    }
  }
  ary
end

n = 10
ary = triangle(n)
ary.each {|row| p row}
p ary.flatten
