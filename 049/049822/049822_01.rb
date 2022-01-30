def A(n, m)
  ary = [[1]]
  (2..n).each{|i|
    a = [1]
    (2..m).each{|j|
      s = 0
      (1..i - 1).each{|k|
        # T(n,k) = Sum_{d|n, d<n} T(n/d-1,k-1)
        s += ary[i / k - 2][j - 2] if i % k == 0 && i / k >= j # i / k - 1 >= j - 1
      }
      a << s
    }
    ary << a
  }
  ary
end

n = 5000
ary = A(n, 3)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1][2].to_i # nillの時0
}
