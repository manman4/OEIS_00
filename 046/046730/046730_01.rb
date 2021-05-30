require 'prime'

def f(n)
  i = 1
  flag = true
  while flag
    i += 1
    j = n - i * i
    flag = false if Math.sqrt(j).to_i ** 2 == j
  end
  j = Math.sqrt(n - i * i).to_i
  i, j = j, i if i % 2 == 0
  return i if (i - j - 1) % 4 == 0
  -i
end

def A046730(n)
  ary = []
  Prime.each(10 ** 6).select{|i| i % 4 == 1}.each{|i| ary << f(i)}
  ary[0..n - 1]
end

n = 10000
ary = A046730(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
