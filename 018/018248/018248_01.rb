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
  str = s.to_s.reverse
end

n = 20
str = A(6, n)
(0..n).each{|i|
  print i
  print ' '
  # nil.to_i‚Í0
  puts str[i].to_i
}
