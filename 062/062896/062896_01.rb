def A(n)
  ary = [[n]]
  cnt = 1
  while ary.size > 0
    f_ary = []
    ary.each{|i|
      i_size = i.size
      (1..i[0] - 1).each{|j|
        a = [j]
        (0..i_size - 1).each{|k| 
          num = i[k] - a[k]
          if num > 0
            a << num
          else
            break
          end
        }
        f_ary << a if a.size == i_size + 1
      }
    }
    ary = f_ary
    
    cnt += (ary.size + ary.select{|i| i == i.reverse}.size) / 2
  end
  cnt
end

# p (1..50).map{|i| A(i)}
(1..200).each{|i| 
  print i
  print ' '
  puts A(i)
}