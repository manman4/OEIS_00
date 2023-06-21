def a(n, r)
  s = 0
  (1..n).each{|i|
    break if i ** r > n
    (1..i).each{|j|
      k = i ** r + j ** r
      break if k > n
      s += 1 if k == n
    }
  }
  s
end

n = 100000000
(1..4).each{|r|
(1..n).each{|i| 
  if a(i, r) == 2
    puts i
    break
  end
  }
}

