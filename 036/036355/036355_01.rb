# T(n,k) is the number of lattice paths from (0,0) to (k,n-k) using steps (1,0),(2,0),(0,1),(0,2).
# Cf. A091533
def A(n, k)
  # 配列を初期化 (k+1)*(n+1) サイズ
  dp = Array.new(k + 1){Array.new(n + 1, 0)}
  dp[0][0] = 1

  (0..k).each{|y|
    (0..n).each{|x|
      next if dp[y][x] == 0
      # (1,0) のステップ
      dp[y][x + 1] += dp[y][x] if x + 1 <= n
      # (2,0) のステップ
      dp[y][x + 2] += dp[y][x] if x + 2 <= n
      # (0,1) のステップ
      dp[y + 1][x] += dp[y][x] if y + 1 <= k
      # (0,2) のステップ
      dp[y + 2][x] += dp[y][x] if y + 2 <= k
    }
  }
  dp[k][n]
end

def T(n, k)
  A(k, n - k)
end

ary = []
(0..10).each{|i|
  (0..i).each{|j|
    ary << T(i, j)
  }
}
p ary