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

n = 500
(0..n).each{|i|
  print i
  print ' '
  puts A(i, 2 * i)
}