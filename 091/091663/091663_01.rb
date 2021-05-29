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
  str = (10 ** (n + 1) - s).to_s.reverse
end

n = 9999
str = A(5, n + 10)
(0..n).each{|i|
  print i
  print ' '
  # nil.to_i‚Í0
  puts str[i].to_i
}
