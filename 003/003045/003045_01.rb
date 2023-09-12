def A(n)
  h = {3 => 1, 4 => 1, 5 => 2, 6 => 2, 7 => 3, 8 => 2, 9 => 2, 10 => 1, 11 => 1}
  a = [1, 2, 3, 4, 5, 6]
  s = 7
  while a.size < n
    if h.include?(s)
      if h[s] == 3
        a.each{|i|
          j = i + s
          if h.include?(j)
            h[j] += 1
          else
            h[j] = 1
          end
        }
        a << s
      end
    end
    s += 1
  end
  a
end

n = 10000
ary = A(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}