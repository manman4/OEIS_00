def A(n)
  cnt = 0
  (0..n).each{|i|
    (i..n - i).each{|j|
      break if i * j > n
      cnt += 1
    }
  }
  cnt
end

n = 9999
(0..n).each{|i|
  print i
  print ' '
  puts A(i)
}

