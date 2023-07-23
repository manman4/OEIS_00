def A(n)
  h = {}
  (1..n).each{|i|
    (i..n).each{|j|
      k = i * i * i + j * j * j
      if h.has_key?(k)
        h[k] += 1
      else
        h[k] = 1
      end
    }
  }
  # 1より大きいものだけを数える
  h.to_a.select{|i| i[1] > 1}.size
end

n = 1000
(1..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}