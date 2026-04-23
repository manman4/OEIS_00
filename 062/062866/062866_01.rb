@max_n = -1
@comb = []
@euler = []
@d_table = []

def ensure_tables(n_max)
  return if n_max <= @max_n

  @comb = Array.new(n_max + 1){Array.new(n_max + 1, 0)}
  0.upto(n_max){|n|
    @comb[n][0] = 1
    @comb[n][n] = 1
    1.upto(n - 1){|k|
      @comb[n][k] = @comb[n - 1][k - 1] + @comb[n - 1][k]
    }
  }

  @euler = Array.new(n_max + 1){Array.new(n_max + 1, 0)}
  @euler[0][0] = 1
  1.upto(n_max){|n|
    0.upto(n - 1){|k|
      left = (k > 0) ? (n - k) * @euler[n - 1][k - 1] : 0
      right = (k + 1) * @euler[n - 1][k]
      @euler[n][k] = left + right
    }
  }

  # d(n, k): derangement のうち、pi(i) > i がちょうど k 個
  @d_table = Array.new(n_max + 1){Array.new(n_max + 1, 0)}
  0.upto(n_max){|n|
    0.upto(n){|k|
      s = 0
      0.upto(n){|j|
        term = @comb[n][j] * @euler[n - j][k]
        s += j.even? ? term : -term
      }
      @d_table[n][k] = s
    }
  }

  @max_n = n_max
end

def d(n, k)
  return 1 if n == 0 && k == 0
  return 0 if n < 0 || k < 0 || k >= n

  ensure_tables(n)
  @d_table[n][k]
end

def a(n, kk)
  return 0 if n < 0

  ensure_tables(n)

  # a(n, K) = sum_{f+e+d=n, e-d=K} C(n, f) * d(e+d, e)
  s = 0
  0.upto(n){|m|
    next if (m + kk).odd?

    e = (m + kk) / 2
    next if e < 0 || e > m

    s += @comb[n][m] * d(m, e)
  }
  s
end

def trimmed_row(n)
  row = (-n..n).map{|kk| a(n, kk)}
  left = row.index {|x| x != 0}
  return [0] if left.nil?

  right = row.rindex {|x| x != 0}
  row[left..right]
end

cnt = 0
n_max = 100
0.upto(n_max){|n|
  trimmed_row(n).each{|x|
    print cnt
    print ' '
    puts x
    cnt += 1
  }
}
