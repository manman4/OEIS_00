def A(n, mod, k)
  s = 0
  (1..n).each{|i|
    if n % i == 0
      s += i if (n / i) % mod == k
    end
  }
  s
end

n = 10000
(1..n).each{|i|
  print i
  print ' '
  puts A(i, 4, 3)
}

