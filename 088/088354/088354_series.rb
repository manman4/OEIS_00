class GeneralizedContinuedFraction
  # --- べき級数演算ライブラリ ---
  # 係数列を多項式として扱い、加算・除算・乗算を行う
  module Series
    # 多項式の加算: c[i] = a[i] + b[i]
    def self.add(a, b, n)
      Array.new(n + 1){|i| (a[i] || 0) + (b[i] || 0)}
    end

    # 多項式の除算: num / den を n 次まで求める
    def self.div(num, den, n)
      result = Array.new(n + 1, 0)
      d0 = Rational(den[0] || 1)

      (0..n).each{|i|
        s = (1..i).sum{|j| (den[j] || 0) * result[i - j]}
        result[i] = (Rational(num[i] || 0) - s) / d0
      }

      result
    end

    # 多項式の乗算: a * b を n 次まで求める
    def self.poly_mul(a, b, n)
      result = Array.new(n + 1, 0)

      a.each_with_index{|ca, i|
        next if ca.nil? || ca == 0 || i > n

        b.each_with_index{|cb, j|
          next if cb.nil? || cb == 0 || (i + j) > n

          result[i + j] += ca * cb
        }
      }

      result
    end
  end
end
