require 'prime'
def A(k, n)
  ary = []
  str = k.to_s
  s = 1
  while ary.size < n
    if !ary.include?(s)
      if (str + s.to_s).to_i.prime?
        ary << s
      end
    end
    s += 1
  end
  ary
end

n = 10000
ary = A(3, n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
