# Number of ordered integer pairs (i,j) with 0 <= i <= j <= n such that i+j <= n and i*j <= n.

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

