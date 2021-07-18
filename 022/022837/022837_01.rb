require 'prime'

@ary = Prime.each(10 ** 6).to_a

def A(n)
   s = 1
   a = [1]
   (1..n).each{|i|
     j = @ary[i - 1]
     if s < j
       s += j
     else
       s -= j
     end
     a << s
   }
   a
end

n = 10000
ary = A(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
