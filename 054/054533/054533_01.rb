# mŸˆÈ‰º‚ğæ‚èo‚·
def mul(f_ary, b_ary, m)
  s1, s2 = f_ary.size, b_ary.size
  ary = Array.new(s1 + s2 - 1, 0)
  (0..s1 - 1).each{|i|
    (0..s2 - 1).each{|j|
      ary[i + j] += f_ary[i] * b_ary[j]
    }
  }
  ary[0..m]
end

def A054533(n)
  s_ary = [1]
  ary = [1]
  (1..n - 1).each{|i|
    b_ary = Array.new(i + 1, 0)
    b_ary[0], b_ary[-1] = 1, -1
    ary = mul(ary, b_ary, i * (i + 1) / 2)
    ary0 = []
    (0..i).each{|j|
      s = 0
      j.step(ary.size - 1, i + 1){|k|
        s += ary[k]
      }
      ary0 << s
    }
    s_ary << ary0.reverse
  }
  s_ary
end

n = 140
ary = A054533(n).flatten
(1..ary.size).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
