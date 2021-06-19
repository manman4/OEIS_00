require 'bigdecimal'

def A(n)
  cnt = 0
  (1..n / 2).each{|i|
    j = n - 2 * i
    if j > 0 && 2 * i > j
      m = (i * i - j * j / 4r) * j * j / 4r
      if m.denominator == 1
        m = m.to_i
        if (BigDecimal.new(m.to_s).sqrt(100)).to_i ** 2 == m
          cnt += 1
        end
      end
    end
  }
  cnt
end

n = 1000
ary = (1..n).map{|i| A(i)}
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
