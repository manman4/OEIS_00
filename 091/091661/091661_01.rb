def A(s, n)
  n.times{|i|
    m = 10 ** (i + 1)
    (0..9).each{|j|
      k = j * m + s
      if (k ** 2 - k) % (m * 10) == 0
        s = k
        break
      end
    }
  }
  s
end
def A091661(n)
  str = (10 ** (n + 1) + A(5, n) - A(6, n)).to_s.reverse
  (0..n).map{|i| str[i].to_i}
end
p A091661(9999)
