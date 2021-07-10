def f(m, n)
  (0..n).each{|i|
    print i
    print ' '
    puts i.to_s.split('').map(&:to_i).inject(0){|s, i| s + i ** m}
  }
end
f(5, 100)
