def a(n)
  a = [0, 2, 4, 6, 8] * 2
  ary = [1]
  puts '1 1'
  cnt, s, max = 1, 1, 1
  while cnt < n
    if ary[0] > 4
      ary.unshift(0)
      s += 1
    end
    # 1000 digits
    break if s > 1000
    (0..s - 1).each{|i|
      j = ary[i]
      ary[i - 1] += 1 if j > 4
      ary[i] = a[j]
    }
    sum = ary.inject(:+)
    if sum > max
      max = sum
      cnt += 1
      print cnt
      print ' '
      puts ary.join.to_i
    end
  end
end
a(1000)
