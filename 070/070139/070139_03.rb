def A0(n)
  cnt = 0
  (1..n / 3).each{|i|
    (i..n / 2).each{|j|
      k = n - i - j
      if k >= j && i + j > k
        s = (i + j + k) / 2r
        m = s * (s - i) * (s - j) * (s - k)
        if m.denominator == 1
          m = m.to_i
          if Math.sqrt(m).to_i ** 2 == m
            cnt += 1
          end
        end
      end
    }
  }
  cnt
end

def A1(n)
  cnt = 0
  (1..n / 3).each{|i|
    (i + 1..n / 2).each{|j|
      k = n - i - j
      if k > j && i + j > k
        s = (i + j + k) / 2r
        m = s * (s - i) * (s - j) * (s - k)
        if m.denominator == 1
          m = m.to_i
          if Math.sqrt(m).to_i ** 2 == m
            cnt += 1
          end
        end
      end
    }
  }
  cnt
end

n = 1000
ary0 = (1..n).map{|i| A0(i)}
ary1 = (1..n).map{|i| A1(i)}
(1..n).each{|i|
  j = ary0[i - 1] - ary1[i- 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
