def partition(n, min, max)
  return [[]] if n == 0
  [max, n].min.downto(min).flat_map{|i| partition(n - i, min, i).map{|rest| [i, *rest]}}
end
def A058360(n)
  print 1
  print ' '
  puts 1
  (2..n).each{|m|
    cnt = 0
    partition(m, 1, m).each{|ary|
      cnt += 1 if ary.inject(0){|s, i| s + 1 / i.to_r}.denominator == 1
    }
    print m
    print ' '
    puts cnt
  }
end
A058360(61)
