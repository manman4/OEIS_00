def A(n)
  cnt = 0
  (1..n).each{|i|
     k = n - i
     if k > i
       if k % i > 0
         cnt += 1
       end
     end
  }
  cnt
end

n = 10000
(1..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
