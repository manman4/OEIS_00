def A066482(n)
  ary = []
  (2..n).each{|i|
    if i % 2 == 0
      ary << i if n % i == i / 2
    else
      ary << i if (n % i == (i - 1) / 2) || (n % i == (i + 1) / 2)
    end
  }
  ary.min
end

(3..10000).each{|i|
  print i
  print ' '
  puts A066482(i)
}
