def A(n)
  a = [1, 2, 3, 4, 5, 6]
  while a.size < n
    s = a.size
    h = {}
    (1..s - 1).each{|i|
      (i + 1..s).each{|j|
        k = a[i - 1] + a[j - 1]
        if h.include?(k)
          h[k] += 1
        else
          h[k] = 1
        end
      }
    }
    # hのvalueが3となるkeyを昇順に並べる
    a << h.select{|k, v| v == 3}.map{|k, v| k}.select{|i| i > a[-1]}.sort[0]
  end
  a
end

n = 2000
ary = A(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}

# 1993 30357
# 1994 30388
# 1995 30399
# 1996 30406
# 1997 30435
# 1998 30457
# 1999 30484
# 2000 30511
