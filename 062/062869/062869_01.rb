def A(n, k)
  dp = Array.new(n+1){Array.new(n+1){Array.new(k+1, 0)}}
  dp[0][0][0] = 1
  n.times{|i|
    (i+1).times{|j|
      (k+1).times{|l|
        next if k < l+2*(i-j)
        dp[i+1][j+1][l+2*(i-j)] += dp[i][j][l]
        dp[i+1][j  ][l+2*(i-j)] += dp[i][j][l]
        dp[i+1][j+1][l+2*(i-j)] += dp[i][j][l] * 2 * (i-j)
        dp[i+1][j+2][l+2*(i-j)] += dp[i][j][l] * (i-j) * (i-j) if j+2 <= n
      }
    }
  }
  dp[n][n][k]
end

def A062869(n)
  (0..n).map{|i| (0..i * i / 4).map{|j| A(i, 2 * j)}}.flatten
end

n = 9
p ary = A062869(n)
# (0..ary.size - 1).each{|i|
#   print i
#   print ' '
#   puts ary[i]
# }