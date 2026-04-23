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
  m = kk.abs
  m += 1 if (m + kk).odd?
  while m <= n
    e = (m + kk) / 2
    s += @comb[n][m] * @d_table[m][e]
    m += 2
  end
  s
end

def trimmed_row(n)
  row = (-n..n).map{|kk| a(n, kk)}
  left = row.index{|x| x != 0}
  return [0] if left.nil?

  right = row.rindex{|x| x != 0}
  row[left..right]
end

def column_values(kk, terms, start_n = 0)
  ensure_tables(start_n + terms - 1)
  vals = []
  n = start_n
  while vals.size < terms
    vals << a(n, kk)
    n += 1
  end
  vals
end


k = 0
terms = 451
start_n = 0
column_values(k, terms, start_n).each_with_index{|v, i|
  j = v
  break if j.to_s.size > 1000
  puts "#{start_n + i} #{v}"
}

